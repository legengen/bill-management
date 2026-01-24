#include "BillQueryScreen.h"
#include "App.h"
#include "Router.h"
#include "Session.h"
#include "UserInfoBar.h"
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
    
    // 查询相关
    bool is_filtered = false;
    std::string filter_desc;
    
    // 弹窗相关
    QueryDialogType dialog_type = QueryDialogType::None;
    
    // 事件查询 - 改为下拉列表
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
    
    void LoadAllBills() {
        auto& bill_service = App::Instance().GetBillService();
        bills = bill_service.queryByPhone("");
        is_filtered = false;
        filter_desc.clear();
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
        
        is_filtered = true;
        filter_desc = "事项: " + selected_event.name;
        current_page = 0;
        selected_row = 0;
        dialog_type = QueryDialogType::None;
        
        if (bills.empty()) {
            DialogManager::Instance().ShowInfo("未找到相关账单");
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
            
            is_filtered = true;
            filter_desc = "日期: " + year_str + "-" + month_str + "-" + day_str;
            current_page = 0;
            selected_row = 0;
            dialog_type = QueryDialogType::None;
            
            if (bills.empty()) {
                DialogManager::Instance().ShowInfo("该日期无账单记录");
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

        // 首先尝试精确匹配
        auto exact_user = user_service.GetUserByPhone(phone);
        if (exact_user.has_value()) {
            // 精确匹配成功，直接查询该用户的账单
            bills = bill_service.queryByPhone(phone);
            is_filtered = true;
            filter_desc = "手机号: " + phone;
            current_page = 0;
            selected_row = 0;
            dialog_type = QueryDialogType::None;
            
            if (bills.empty()) {
                DialogManager::Instance().ShowInfo("该用户无账单记录");
            }
            return;
        }

        // 精确匹配失败，尝试模糊匹配
        auto matched_users = user_service.QueryUserByPhone(phone);
        
        if (matched_users.empty()) {
            DialogManager::Instance().ShowError("未找到匹配的用户");
            return;
        }
        
        // 收集所有匹配用户的账单
        bills.clear();
        std::vector<std::string> matched_phones;
        
        for (const auto& user : matched_users) {
            matched_phones.push_back(user.phone);
            auto user_bills = bill_service.queryByPhone(user.phone);
            bills.insert(bills.end(), user_bills.begin(), user_bills.end());
        }
        
        is_filtered = true;
        
        // 构建筛选描述
        if (matched_users.size() == 1) {
            filter_desc = "手机号: " + matched_phones[0];
        } else {
            filter_desc = "手机号包含 \"" + phone + "\" (匹配 " + 
                        std::to_string(matched_users.size()) + " 个用户)";
        }
        
        current_page = 0;
        selected_row = 0;
        dialog_type = QueryDialogType::None;
        
        if (bills.empty()) {
            DialogManager::Instance().ShowInfo("匹配的用户均无账单记录");
        } else {
            DialogManager::Instance().ShowSuccess(
                "找到 " + std::to_string(matched_users.size()) + " 个匹配用户，共 " + 
                std::to_string(bills.size()) + " 条账单"
            );
        }
    }
    
    void OpenRemarkDialog() {
        auto* bill = GetSelectedBill();
        if (bill) {
            remark_bill_index = current_page * page_size + selected_row;
            new_remark = bill->description;
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
        bill.description = new_remark;
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
    
    // 主页面按钮
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
    
    auto prev_btn = Button("◀", [state] {
        state->PrevPage();
    });
    
    auto next_btn = Button("▶", [state] {
        state->NextPage();
    });
    
    auto return_btn = Button("返回", [] {
        Router::Instance().NavigateTo(Route::BillManage);
    });
    
    // 事项查询弹窗组件 - 使用下拉列表
    auto event_dropdown = Dropdown(&state->event_names, &state->selected_event_index);
    auto event_confirm_btn = Button("确认", [state] {
        state->QueryByEvent();
    });
    auto event_cancel_btn = Button("取消", [state] {
        state->CloseDialog();
    });
    
    // 时间查询弹窗组件
    auto year_input = Input(&state->year_str, "年");
    auto month_input = Input(&state->month_str, "月");
    auto day_input = Input(&state->day_str, "日");
    auto time_confirm_btn = Button("确认", [state] {
        state->QueryByTime();
    });
    auto time_cancel_btn = Button("取消", [state] {
        state->CloseDialog();
    });
    
    // 手机号查询弹窗组件
    auto phone_input = Input(&state->phone, "手机号");
    auto phone_confirm_btn = Button("确认", [state] {
        state->QueryByPhone();
    });
    auto phone_cancel_btn = Button("取消", [state] {
        state->CloseDialog();
    });
    
    // 批注弹窗组件
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
    
    // 主容器
    auto main_container = Container::Vertical({
        Container::Horizontal({prev_btn, next_btn}),
        Container::Horizontal({show_all_btn, by_event_btn, by_time_btn, by_phone_btn, return_btn}),
    });
    
    // 事项弹窗容器
    auto event_dialog_container = Container::Vertical({
        event_dropdown,
        Container::Horizontal({event_confirm_btn, event_cancel_btn}),
    });
    
    // 时间弹窗容器
    auto time_dialog_container = Container::Vertical({
        Container::Horizontal({year_input, month_input, day_input}),
        Container::Horizontal({time_confirm_btn, time_cancel_btn}),
    });
    
    // 手机号弹窗容器
    auto phone_dialog_container = Container::Vertical({
        phone_input,
        Container::Horizontal({phone_confirm_btn, phone_cancel_btn}),
    });
    
    // 批注弹窗容器
    auto remark_dialog_container = Container::Vertical({
        remark_input,
        Container::Horizontal({remark_confirm_btn, remark_cancel_btn}),
    });
    
    auto tab_index = std::make_shared<int>(0);
    auto container = Container::Tab({
        main_container,           // 0
        event_dialog_container,   // 1
        time_dialog_container,    // 2
        phone_dialog_container,   // 3
        remark_dialog_container,  // 4
    }, tab_index.get());
    
    // 键盘事件处理
    container = CatchEvent(container, [state](Event event) {
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

    return Renderer(container, [=] {
        // 设置 tab 索引
        switch (state->dialog_type) {
            case QueryDialogType::None: *tab_index = 0; break;
            case QueryDialogType::ByEvent: *tab_index = 1; break;
            case QueryDialogType::ByTime: *tab_index = 2; break;
            case QueryDialogType::ByPhone: *tab_index = 3; break;
            case QueryDialogType::Remark: *tab_index = 4; break;
        }
        
        // ==================== 构建主页面 ====================
        auto title = text("账单查询") | bold | center;
        auto user_info = RenderUserInfoBar(PageType::Home);
        
        Element filter_status;
        if (state->is_filtered) {
            filter_status = hbox({
                text("筛选: ") | bold,
                text(state->filter_desc) | color(Color::Green),
                text("  共 " + std::to_string(state->bills.size()) + " 条记录"),
            }) | center;
        } else {
            filter_status = text("显示全部账单，共 " + std::to_string(state->bills.size()) + " 条记录") | center | dim;
        }
        
        // 构建表格
        std::vector<std::vector<std::string>> table_data;
        table_data.push_back({"", "ID", "手机号", "事项", "金额", "备注", "时间"});
        
        auto current_bills = state->GetCurrentPageBills();
        for (int i = 0; i < static_cast<int>(current_bills.size()); i++) {
            const auto& bill = current_bills[i];
            std::string selector = (i == state->selected_row) ? ">" : " ";
            table_data.push_back({
                selector,
                std::to_string(bill.id),
                state->GetUserPhone(bill.owner_id),
                state->GetEventName(bill.event_id),
                state->FormatAmount(bill.amount),
                bill.description,
                state->FormatTime(bill.created_at)
            });
        }
        
        if (current_bills.empty()) {
            table_data.push_back({"", "", "", "暂无账单数据", "", "", ""});
        }
        
        auto table = Table(table_data);
        table.SelectAll().Border(LIGHT);
        table.SelectRow(0).Decorate(bold);
        table.SelectColumn(0).DecorateCells(size(WIDTH, EQUAL, 3));
        table.SelectColumn(1).DecorateCells(size(WIDTH, EQUAL, 6));
        table.SelectColumn(2).DecorateCells(size(WIDTH, EQUAL, 13));
        table.SelectColumn(3).DecorateCells(size(WIDTH, EQUAL, 10));
        table.SelectColumn(4).DecorateCells(size(WIDTH, EQUAL, 10));
        table.SelectColumn(5).DecorateCells(size(WIDTH, EQUAL, 12));
        table.SelectColumn(6).DecorateCells(size(WIDTH, EQUAL, 16));
        table.SelectAll().DecorateCells(center);
        
        if (state->selected_row >= 0 && state->selected_row < static_cast<int>(current_bills.size())) {
            table.SelectRow(state->selected_row + 1).Decorate(bgcolor(Color::Blue));
        }
        
        auto table_element = table.Render();
        
        auto page_info = text("当前页" + std::to_string(state->current_page + 1) + 
                              "/" + "总页数" + std::to_string(state->GetTotalPages()) +
                              " 按←→翻页") | center;
        
        auto page_buttons = hbox({
            filler(),
            prev_btn->Render() | size(WIDTH, EQUAL, 5),
            text("  "),
            next_btn->Render() | size(WIDTH, EQUAL, 5),
            filler(),
        });
        
        auto bottom_buttons = hbox({
            filler(),
            show_all_btn->Render() | size(WIDTH, EQUAL, 10),
            text(" "),
            by_event_btn->Render() | size(WIDTH, EQUAL, 10),
            text(" "),
            by_time_btn->Render() | size(WIDTH, EQUAL, 10),
            text(" "),
            by_phone_btn->Render() | size(WIDTH, EQUAL, 12),
            text(" "),
            return_btn->Render() | size(WIDTH, EQUAL, 8),
            filler(),
        });
        
        auto content = vbox({
            title,
            user_info,
            text(""),
            filter_status,
            text(""),
            table_element | center,
            text(""),
            page_info,
            text(""),
            bottom_buttons,
        });
        
        auto main_page = vbox({
            filler() | size(HEIGHT, EQUAL, 1),
            hbox({
                filler() | size(WIDTH, EQUAL, 2),
                content | flex,
                filler() | size(WIDTH, EQUAL, 2),
            }),
            filler() | size(HEIGHT, EQUAL, 1),
        }) | border;
        
        // ==================== 弹窗渲染 ====================
        
        // 事项查询弹窗 - 使用下拉列表
        if (state->dialog_type == QueryDialogType::ByEvent) {
            auto dialog_box = vbox({
                text("按事项查询") | bold | center,
                separator(),
                text(""),
                hbox({
                    text("选择事项: ") | vcenter,
                    event_dropdown->Render() | size(WIDTH, EQUAL, 20),
                }) | center,
                text(""),
                separator(),
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
        
        // 时间查询弹窗
        if (state->dialog_type == QueryDialogType::ByTime) {
            auto dialog_box = vbox({
                text("按时间查询") | bold | center,
                separator(),
                text(""),
                text("查询指定日期的账单") | center | dim,
                text(""),
                hbox({
                    text("查询日期: ") | center,
                    year_input->Render() | size(WIDTH, EQUAL, 6) | border,
                    text("-") | center,
                    month_input->Render() | size(WIDTH, EQUAL, 4) | border,
                    text("-") | center,
                    day_input->Render() | size(WIDTH, EQUAL, 4) | border,
                }) | center,
                text(""),
                separator(),
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
        
        // 手机号查询弹窗
        if (state->dialog_type == QueryDialogType::ByPhone) {
            auto dialog_box = vbox({
                text("按手机号查询") | bold | center,
                separator(),
                text(""),
                hbox({
                    text("手机号: ") | center,
                    phone_input->Render() | size(WIDTH, EQUAL, 15) | border,
                }) | center,
                text(""),
                separator(),
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
        
        // 批注弹窗
        if (state->dialog_type == QueryDialogType::Remark) {
            auto* bill = (state->remark_bill_index >= 0 && 
                         state->remark_bill_index < static_cast<int>(state->bills.size())) 
                         ? &state->bills[state->remark_bill_index] : nullptr;
            
            std::string bill_info = bill ? 
                ("账单ID: " + std::to_string(bill->id) + "  金额: " + state->FormatAmount(bill->amount)) : "";
            
            auto dialog_box = vbox({
                text("编辑批注") | bold | center,
                separator(),
                text(""),
                text(bill_info) | center | dim,
                text(""),
                hbox({
                    text("批注: ") | center,
                    remark_input->Render() | size(WIDTH, EQUAL, 20) | border,
                }) | center,
                text(""),
                separator(),
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
        
        // 无弹窗时显示主页面
        return main_page;
    });
}