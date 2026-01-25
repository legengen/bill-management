#include "BillQueryScreen.h"
#include "App.h"
#include "Router.h"
#include "Session.h"
#include "UserInfoBar.h"
#include "PageLayout.h"
#include "Dialog.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <memory>
#include <vector>
#include <iomanip>
#include <sstream>
#include <ctime>

using namespace ftxui;

enum class QueryDialogType {
    None,
    ByEvent,
    ByTime,
    ByPhone,
    Remark
};

struct BillQueryState {
    std::vector<model::Bill> bills;
    int current_page = 0;
    int page_size = 3;
    int selected_row = 0;
    
    // 弹窗相关
    QueryDialogType dialog_type = QueryDialogType::None;
    
    // 事件查询
    std::vector<model::Event> all_events;
    std::vector<std::string> event_names;
    int selected_event_index = 0;
    
    // 时间查询
    std::string year_str;
    std::string month_str;
    std::string day_str;
    
    // 手机号查询
    std::string phone;
    
    // 批注弹窗
    int remark_bill_index = -1;
    std::string new_remark;
    std::string current_annotation;
    
    void LoadAllBills() {
        auto& bill_service = App::Instance().GetBillService();
        bills = bill_service.queryByPhone("");
        current_page = 0;
        selected_row = 0;
    }
    
    void LoadAllEvents() {
        auto& event_service = App::Instance().GetEventService();
        all_events = event_service.GetAllEvents();
        event_names.clear();
        for (const auto& e : all_events) {
            event_names.push_back(e.name);
        }
        if (event_names.empty()) {
            event_names.push_back("(无可用事项)");
        }
        selected_event_index = 0;
    }
    
    void QueryByEvent() {
        if (all_events.empty() || selected_event_index < 0 || 
            selected_event_index >= static_cast<int>(all_events.size())) {
            DialogManager::Instance().ShowError("请选择有效事项");
            return;
        }
        
        auto& selected_event = all_events[selected_event_index];
        auto& bill_service = App::Instance().GetBillService();
        bills = bill_service.queryByEvent(0, selected_event.id);
        
        current_page = 0;
        selected_row = 0;
        dialog_type = QueryDialogType::None;
        
        if (bills.empty()) {
            DialogManager::Instance().ShowInfo("未找到相关账单");
        } else {
            DialogManager::Instance().ShowSuccess("找到 " + std::to_string(bills.size()) + " 条账单");
        }
    }
    
    void QueryByTime() {
        if (year_str.empty() || month_str.empty() || day_str.empty()) {
            DialogManager::Instance().ShowError("请输入完整日期");
            return;
        }
        
        try {
            int year = std::stoi(year_str);
            int month = std::stoi(month_str);
            int day = std::stoi(day_str);
            
            std::tm start_tm = {};
            start_tm.tm_year = year - 1900;
            start_tm.tm_mon = month - 1;
            start_tm.tm_mday = day;
            start_tm.tm_hour = 0;
            start_tm.tm_min = 0;
            start_tm.tm_sec = 0;
            std::time_t start_time = std::mktime(&start_tm);
            
            std::tm end_tm = start_tm;
            end_tm.tm_hour = 23;
            end_tm.tm_min = 59;
            end_tm.tm_sec = 59;
            std::time_t end_time = std::mktime(&end_tm);
            
            auto& bill_service = App::Instance().GetBillService();
            bills = bill_service.QueryByTime(0, start_time, end_time);
            
            current_page = 0;
            selected_row = 0;
            dialog_type = QueryDialogType::None;
            
            if (bills.empty()) {
                DialogManager::Instance().ShowInfo("该日期无账单记录");
            } else {
                DialogManager::Instance().ShowSuccess("找到 " + std::to_string(bills.size()) + " 条账单");
            }
        } catch (...) {
            DialogManager::Instance().ShowError("日期格式错误");
        }
    }
    
    void QueryByPhone() {
        if (phone.empty()) {
            DialogManager::Instance().ShowError("请输入手机号");
            return;
        }
        
        auto& user_service = App::Instance().GetUserService();
        auto& bill_service = App::Instance().GetBillService();

        auto exact_user = user_service.GetUserByPhone(phone);
        if (exact_user.has_value()) {
            bills = bill_service.queryByPhone(phone);
            current_page = 0;
            selected_row = 0;
            dialog_type = QueryDialogType::None;
            
            if (bills.empty()) {
                DialogManager::Instance().ShowInfo("该用户无账单记录");
            } else {
                DialogManager::Instance().ShowSuccess("找到 " + std::to_string(bills.size()) + " 条账单");
            }
            return;
        }

        auto matched_users = user_service.QueryUserByPhone(phone);
        
        if (matched_users.empty()) {
            DialogManager::Instance().ShowError("未找到匹配的用户");
            return;
        }
        
        bills.clear();
        for (const auto& user : matched_users) {
            auto user_bills = bill_service.queryByPhone(user.phone);
            bills.insert(bills.end(), user_bills.begin(), user_bills.end());
        }
        
        current_page = 0;
        selected_row = 0;
        dialog_type = QueryDialogType::None;
        
        if (bills.empty()) {
            DialogManager::Instance().ShowInfo("匹配的用户均无账单记录");
        } else {
            DialogManager::Instance().ShowSuccess(
                "找到 " + std::to_string(matched_users.size()) + " 个用户，共 " + 
                std::to_string(bills.size()) + " 条账单"
            );
        }
    }
    
    void OpenRemarkDialog() {
        auto* bill = GetSelectedBill();
        if (bill) {
            remark_bill_index = current_page * page_size + selected_row;
            dialog_type = QueryDialogType::Remark;
        }
    }
    
    bool UpdateRemark() {
        if (remark_bill_index < 0 || remark_bill_index >= static_cast<int>(bills.size())) {
            return false;
        }
        
        auto& bill = bills[remark_bill_index];
        auto& bill_service = App::Instance().GetBillService();
        
        model::Annotation annotation;
        annotation.id = 0;
        annotation.bill_id = bill.id;
        annotation.content = new_remark;
        annotation.created_at = std::time(nullptr);
        
        bill_service.annotateBill(bill.id, annotation);
        return true;
    }
    
    int GetTotalPages() const {
        if (bills.empty()) return 1;
        return (bills.size() + page_size - 1) / page_size;
    }
    
    std::vector<model::Bill> GetCurrentPageBills() const {
        std::vector<model::Bill> result;
        int start = current_page * page_size;
        int end = std::min(start + page_size, static_cast<int>(bills.size()));
        
        for (int i = start; i < end; i++) {
            result.push_back(bills[i]);
        }
        return result;
    }
    
    void NextPage() {
        if (current_page < GetTotalPages() - 1) {
            current_page++;
            selected_row = 0;
        }
    }
    
    void PrevPage() {
        if (current_page > 0) {
            current_page--;
            selected_row = 0;
        }
    }
    
    void SelectNextRow() {
        auto page_bills = GetCurrentPageBills();
        if (selected_row < static_cast<int>(page_bills.size()) - 1) {
            selected_row++;
        }
    }
    
    void SelectPrevRow() {
        if (selected_row > 0) {
            selected_row--;
        }
    }
    
    model::Bill* GetSelectedBill() {
        int global_index = current_page * page_size + selected_row;
        if (global_index >= 0 && global_index < static_cast<int>(bills.size())) {
            return &bills[global_index];
        }
        return nullptr;
    }
    
    void OpenEventDialog() {
        LoadAllEvents();
        selected_event_index = 0;
        dialog_type = QueryDialogType::ByEvent;
    }
    
    void OpenTimeDialog() {
        year_str.clear();
        month_str.clear();
        day_str.clear();
        dialog_type = QueryDialogType::ByTime;
    }
    
    void OpenPhoneDialog() {
        phone.clear();
        dialog_type = QueryDialogType::ByPhone;
    }
    
    void CloseDialog() {
        dialog_type = QueryDialogType::None;
        year_str.clear();
        month_str.clear();
        day_str.clear();
        phone.clear();
        new_remark.clear();
        remark_bill_index = -1;
    }
    
    std::string FormatTime(model::Timestamp ts) const {
        std::time_t time = static_cast<std::time_t>(ts);
        std::tm* tm = std::localtime(&time);
        std::ostringstream oss;
        oss << std::put_time(tm, "%Y-%m-%d %H:%M");
        return oss.str();
    }
    
    std::string FormatAmount(double amount) const {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << amount;
        return oss.str();
    }
    
    std::string GetEventName(int event_id) const {
        auto& event_service = App::Instance().GetEventService();
        auto event = event_service.QueryById(event_id);
        return event.has_value() ? event->name : "未知";
    }
    
    std::string GetUserPhone(int user_id) const {
        auto& user_service = App::Instance().GetUserService();
        auto user = user_service.GetUser(user_id);
        return user.has_value() ? user->phone : "未知";
    }
};

ftxui::Component CreateBillQueryScreen() {
    auto state = std::make_shared<BillQueryState>();
    state->LoadAllBills();
    
    auto tab_index = std::make_shared<int>(0);

    // ==================== 主页面按钮 ====================
    auto show_all_btn = Button("显示全部", [state] {
        state->LoadAllBills();
    });
    
    auto by_event_btn = Button("按事项", [state] {
        state->OpenEventDialog();
    });
    
    auto by_time_btn = Button("按时间", [state] {
        state->OpenTimeDialog();
    });
    
    auto by_phone_btn = Button("按手机号", [state] {
        state->OpenPhoneDialog();
    });
    
    auto remark_btn = Button("编辑批注", [state] {
        if (state->GetSelectedBill()) {
            state->OpenRemarkDialog();
        } else {
            DialogManager::Instance().ShowError("请先选择账单");
        }
    });
    
    auto return_btn = Button("返回", [] {
        Router::Instance().NavigateTo(Route::BillManage);
    });
    
    // ==================== 事项查询弹窗 ====================
    auto event_dropdown = Dropdown(&state->event_names, &state->selected_event_index);
    auto event_confirm_btn = Button("确认", [state] {
        state->QueryByEvent();
    });
    auto event_cancel_btn = Button("取消", [state] {
        state->CloseDialog();
    });
    
    auto event_dialog_container = Container::Vertical({
        event_dropdown,
        Container::Horizontal({event_confirm_btn, event_cancel_btn}),
    });
    
    // ==================== 时间查询弹窗 ====================
    auto year_input = Input(&state->year_str, "年");
    auto month_input = Input(&state->month_str, "月");
    auto day_input = Input(&state->day_str, "日");
    auto time_confirm_btn = Button("确认", [state] {
        state->QueryByTime();
    });
    auto time_cancel_btn = Button("取消", [state] {
        state->CloseDialog();
    });
    
    auto time_dialog_container = Container::Vertical({
        Container::Horizontal({year_input, month_input, day_input}),
        Container::Horizontal({time_confirm_btn, time_cancel_btn}),
    });
    
    // ==================== 手机号查询弹窗 ====================
    auto phone_input = Input(&state->phone, "手机号");
    auto phone_confirm_btn = Button("确认", [state] {
        state->QueryByPhone();
    });
    auto phone_cancel_btn = Button("取消", [state] {
        state->CloseDialog();
    });
    
    auto phone_dialog_container = Container::Vertical({
        phone_input,
        Container::Horizontal({phone_confirm_btn, phone_cancel_btn}),
    });
    
    // ==================== 批注弹窗 ====================
    auto remark_input = Input(&state->new_remark, "输入批注");
    auto remark_confirm_btn = Button("确认", [state] {
        if (state->UpdateRemark()) {
            DialogManager::Instance().ShowSuccess("批注已更新");
            state->CloseDialog();
        } else {
            DialogManager::Instance().ShowError("更新失败");
        }
    });
    auto remark_cancel_btn = Button("取消", [state] {
        state->CloseDialog();
    });
    
    auto remark_dialog_container = Container::Vertical({
        remark_input,
        Container::Horizontal({remark_confirm_btn, remark_cancel_btn}),
    });
    
    // ==================== 主容器（Horizontal避免上下键切换按钮） ====================
    auto main_container = Container::Horizontal({
        show_all_btn, 
        by_event_btn, 
        by_time_btn, 
        by_phone_btn, 
        remark_btn,
        return_btn,
    });
    
    // ==================== Tab 容器 ====================
    auto container = Container::Tab({
        main_container,           // 0
        event_dialog_container,   // 1
        time_dialog_container,    // 2
        phone_dialog_container,   // 3
        remark_dialog_container,  // 4
    }, tab_index.get());

    LayoutConfig config;
    config.button_width = 10;
    config.content_padding = 2;
    config.outer_padding = 2;
    config.content_height = 13;

    const int COL_ID = 6;
    const int COL_PHONE = 13;
    const int COL_EVENT = 10;
    const int COL_AMOUNT = 10;
    const int COL_REMARK = 12;
    const int COL_TIME = 17;

    return Renderer(container, [=]() {
        switch (state->dialog_type) {
            case QueryDialogType::None:    *tab_index = 0; break;
            case QueryDialogType::ByEvent: *tab_index = 1; break;
            case QueryDialogType::ByTime:  *tab_index = 2; break;
            case QueryDialogType::ByPhone: *tab_index = 3; break;
            case QueryDialogType::Remark:  *tab_index = 4; break;
        }
        
        auto user_info = RenderUserInfoBar();
        
        // ========== 操作按钮行 ==========
        auto action_row = hbox({
            show_all_btn->Render() | size(WIDTH, EQUAL, 10),
            filler(),
            by_event_btn->Render() | size(WIDTH, EQUAL, 10),
            filler(),
            by_time_btn->Render() | size(WIDTH, EQUAL, 10),
            filler(),
            by_phone_btn->Render() | size(WIDTH, EQUAL, 12),
        }) | center;
        
        // ========== 构建表格 ==========
        std::vector<std::vector<std::string>> table_data;
        table_data.push_back({"ID", "手机号", "事项", "金额", "备注", "时间"});
        
        auto current_bills = state->GetCurrentPageBills();
        for (const auto& bill : current_bills) {
            table_data.push_back({
                std::to_string(bill.id),
                state->GetUserPhone(bill.owner_id),
                state->GetEventName(bill.event_id),
                state->FormatAmount(bill.amount),
                bill.description,
                state->FormatTime(bill.created_at)
            });
        }
        
        // 补充空行
        while (table_data.size() < static_cast<size_t>(state->page_size + 1)) {
            table_data.push_back({"", "", "", "", "", ""});
        }
        
        auto table = Table(table_data);
        
        table.SelectAll().SeparatorVertical(LIGHT);
        table.SelectAll().SeparatorHorizontal(LIGHT);
        table.SelectAll().Border(LIGHT);
        
        table.SelectColumn(0).DecorateCells(size(WIDTH, EQUAL, COL_ID));
        table.SelectColumn(1).DecorateCells(size(WIDTH, EQUAL, COL_PHONE));
        table.SelectColumn(2).DecorateCells(size(WIDTH, EQUAL, COL_EVENT));
        table.SelectColumn(3).DecorateCells(size(WIDTH, EQUAL, COL_AMOUNT));
        table.SelectColumn(4).DecorateCells(size(WIDTH, EQUAL, COL_REMARK));
        table.SelectColumn(5).DecorateCells(size(WIDTH, EQUAL, COL_TIME));
        
        table.SelectAll().DecorateCells(center);
        
        table.SelectRow(0).Decorate(bold);
        table.SelectRow(0).DecorateCells(bgcolor(Color::Blue));
        
        if (state->selected_row >= 0 && state->selected_row < static_cast<int>(current_bills.size())) {
            table.SelectRow(state->selected_row + 1).DecorateCells(bgcolor(Color::GrayDark));
        }
        
        auto table_element = table.Render();
        
        // ========== 分页信息 ==========
        auto page_info = hbox({
            text("第 " + std::to_string(state->current_page + 1) + 
                 "/" + std::to_string(state->GetTotalPages()) + " 页") | center,
            filler() | size(WIDTH, EQUAL, 30),
            text("← → 翻页"),
        }) | center;
        
        // ========== 内容区域 ==========
        auto content_area = vbox({
            table_element | center,
            text(""),
            page_info,
        });
        
        // ========== 底部按钮 ==========
        auto button_area = hbox({
            action_row,
            filler(),
            return_btn->Render() | size(WIDTH, EQUAL, config.button_width),
        });
        
        auto main_page = CreatePageLayout(user_info, content_area, button_area, config);
        
        // ==================== 弹窗渲染 ====================
        if (state->dialog_type == QueryDialogType::ByEvent) {
            auto dialog_box = vbox({
                hbox({
                    text("选择事项: ") | vcenter,
                    event_dropdown->Render() | size(WIDTH, EQUAL, 20),
                }) | center,
                hbox({
                    filler(),
                    event_confirm_btn->Render() | size(WIDTH, EQUAL, 10),
                    text("  "),
                    event_cancel_btn->Render() | size(WIDTH, EQUAL, 10),
                    filler(),
                }),
            }) | border | clear_under | center;
            
            return dbox({
                main_page | dim,
                dialog_box,
            });
        }
        
        if (state->dialog_type == QueryDialogType::ByTime) {
            auto dialog_box = vbox({
                hbox({
                    text("日期: ") | vcenter,
                    year_input->Render() | size(WIDTH, EQUAL, 6) | border,
                    text("-") | vcenter,
                    month_input->Render() | size(WIDTH, EQUAL, 4) | border,
                    text("-") | vcenter,
                    day_input->Render() | size(WIDTH, EQUAL, 4) | border,
                }) | center,
                hbox({
                    filler(),
                    time_confirm_btn->Render() | size(WIDTH, EQUAL, 10),
                    text("  "),
                    time_cancel_btn->Render() | size(WIDTH, EQUAL, 10),
                    filler(),
                }),
            }) | border | clear_under | center;
            
            return dbox({
                main_page | dim,
                dialog_box,
            });
        }
        
        if (state->dialog_type == QueryDialogType::ByPhone) {
            auto dialog_box = vbox({
                text(""),
                hbox({
                    text("手机号: ") | vcenter,
                    phone_input->Render() | size(WIDTH, EQUAL, 15) | border,
                }) | center,
                hbox({
                    filler(),
                    phone_confirm_btn->Render() | size(WIDTH, EQUAL, 10),
                    text("  "),
                    phone_cancel_btn->Render() | size(WIDTH, EQUAL, 10),
                    filler(),
                }),
            }) | border | clear_under | center;
            
            return dbox({
                main_page | dim,
                dialog_box,
            });
        }
        
        if (state->dialog_type == QueryDialogType::Remark) {
            auto* bill = (state->remark_bill_index >= 0 && 
                         state->remark_bill_index < static_cast<int>(state->bills.size())) 
                         ? &state->bills[state->remark_bill_index] : nullptr;
            
            std::string bill_info = bill ? 
                ("账单ID: " + std::to_string(bill->id) + "  金额: " + state->FormatAmount(bill->amount)) : "";
            std::string desc_info = bill ? bill->description : "";
            
            auto dialog_box = vbox({
                hbox({
                    text("批注: ") | vcenter,
                    remark_input->Render() | size(WIDTH, EQUAL, 25) | border,
                }) | center,
                hbox({
                    filler(),
                    remark_confirm_btn->Render() | size(WIDTH, EQUAL, 10),
                    text("  "),
                    remark_cancel_btn->Render() | size(WIDTH, EQUAL, 10),
                    filler(),
                }),
            }) | border | clear_under | center;
            
            return dbox({
                main_page | dim,
                dialog_box,
            });
        }
        
        return main_page;
    }) | CatchEvent([state](Event event) {
        if (state->dialog_type != QueryDialogType::None) {
            if (event == Event::Escape) {
                state->CloseDialog();
                return true;
            }
            return false;
        }
        
        if (event == Event::ArrowUp) {
            state->SelectPrevRow();
            return true;
        }
        if (event == Event::ArrowDown) {
            state->SelectNextRow();
            return true;
        }
        if (event == Event::ArrowLeft) {
            state->PrevPage();
            return true;
        }
        if (event == Event::ArrowRight) {
            state->NextPage();
            return true;
        }
        if (event == Event::Return) {
            if (state->GetSelectedBill()) {
                state->OpenRemarkDialog();
                return true;
            }
        }
        return false;
    });
}