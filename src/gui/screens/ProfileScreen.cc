#include "ProfileScreen.h"
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

struct ProfileState {
    std::vector<model::Bill> bills;
    int current_page = 0;
    int page_size = 3;
    
    bool is_filtered = false;
    std::string filter_info;

    std::string query_year;
    std::string query_month;
    std::string query_day;

    std::vector<model::Event> all_events;
    int selected_event_index = 0;
    
    void LoadAllBills() {
        auto& session = Session::Instance();
        auto& bill_service = App::Instance().GetBillService();
        bills = bill_service.queryByPhone(session.GetPhone());
        is_filtered = false;
        filter_info.clear();
        current_page = 0;
    }
    
    void LoadAllEvents() {
        auto& event_service = App::Instance().GetEventService();
        all_events = event_service.GetAllEvents();
        selected_event_index = 0;
    }
    
    std::vector<std::string> GetEventNames() const {
        std::vector<std::string> names;
        for (const auto& event : all_events) {
            names.push_back(event.name);
        }
        if (names.empty()) {
            names.push_back("无可用事件");
        }
        return names;
    }
    
    bool QueryByEvent() {
        if (all_events.empty() || selected_event_index < 0 || 
            selected_event_index >= static_cast<int>(all_events.size())) {
            return false;
        }
        
        int event_id = all_events[selected_event_index].id;
        std::string event_name = all_events[selected_event_index].name;
        
        auto& session = Session::Instance();
        auto& bill_service = App::Instance().GetBillService();
        bills = bill_service.queryByEvent(session.GetUserId(), event_id);
        
        is_filtered = true;
        filter_info = "事件: " + event_name;
        current_page = 0;
        return true;
    }
    
    bool ParseDate(const std::string& year, const std::string& month, const std::string& day, model::Timestamp& out) {
        try {
            int y = std::stoi(year);
            int m = std::stoi(month);
            int d = std::stoi(day);
            
            if (y < 1970 || y > 2100 || m < 1 || m > 12 || d < 1 || d > 31) {
                return false;
            }
            
            std::tm tm = {};
            tm.tm_year = y - 1900;
            tm.tm_mon = m - 1;
            tm.tm_mday = d;
            tm.tm_hour = 0;
            tm.tm_min = 0;
            tm.tm_sec = 0;
            
            out = std::mktime(&tm);
            return out != -1;
        } catch (...) {
            return false;
        }
    }
    
    bool QueryByDate() {
        model::Timestamp start_ts;
        
        if (!ParseDate(query_year, query_month, query_day, start_ts)) {
            return false;
        }
        
        model::Timestamp end_ts = start_ts + 24 * 60 * 60;
        
        auto& session = Session::Instance();
        auto& bill_service = App::Instance().GetBillService();
        bills = bill_service.QueryByTime(session.GetUserId(), start_ts, end_ts);
        
        is_filtered = true;
        filter_info = "日期: " + query_year + "-" + query_month + "-" + query_day;
        current_page = 0;
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
        }
    }
    
    void PrevPage() {
        if (current_page > 0) {
            current_page--;
        }
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
        if (event.has_value()) {
            return event->name;
        }
        return "未知事件";
    }
    
    void InitDateInputs() {
        auto now = std::time(nullptr);
        auto tm = std::localtime(&now);
        
        query_year = std::to_string(tm->tm_year + 1900);
        query_month = std::to_string(tm->tm_mon + 1);
        query_day = std::to_string(tm->tm_mday);
    }
};

// 查询弹窗类型
enum class QueryDialogType {
    None,
    TimeQuery,
    EventQuery,
};

ftxui::Component CreateProfileScreen() {
    auto state = std::make_shared<ProfileState>();
    state->LoadAllBills();
    state->LoadAllEvents();
    state->InitDateInputs();
    
    auto current_dialog = std::make_shared<QueryDialogType>(QueryDialogType::None);
    auto tab_index = std::make_shared<int>(0);
    auto button_index = std::make_shared<int>(0);  // 按钮焦点索引

    // ==================== 时间查询弹窗 ====================
    auto year_input = Input(&state->query_year, "年");
    auto month_input = Input(&state->query_month, "月");
    auto day_input = Input(&state->query_day, "日");

    auto time_confirm_btn = Button("确认", [state, current_dialog] {
        if (state->QueryByDate()) {
            *current_dialog = QueryDialogType::None;
            DialogManager::Instance().ShowSuccess(
                "查询完成，共 " + std::to_string(state->bills.size()) + " 条记录"
            );
        } else {
            DialogManager::Instance().ShowError("日期格式错误");
        }
    });

    auto time_cancel_btn = Button("取消", [current_dialog] {
        *current_dialog = QueryDialogType::None;
    });

    auto time_dialog_container = Container::Vertical({
        Container::Horizontal({year_input, month_input, day_input}),
        Container::Horizontal({time_confirm_btn, time_cancel_btn}),
    });

    // ==================== 事件查询弹窗 ====================
    auto event_names = std::make_shared<std::vector<std::string>>(state->GetEventNames());
    auto event_dropdown = Dropdown(event_names.get(), &state->selected_event_index);

    auto event_confirm_btn = Button("确认", [state, current_dialog] {
        if (state->QueryByEvent()) {
            *current_dialog = QueryDialogType::None;
            DialogManager::Instance().ShowSuccess(
                "查询完成，共 " + std::to_string(state->bills.size()) + " 条记录"
            );
        } else {
            DialogManager::Instance().ShowError("请选择有效事件");
        }
    });

    auto event_cancel_btn = Button("取消", [current_dialog] {
        *current_dialog = QueryDialogType::None;
    });

    auto event_dialog_container = Container::Vertical({
        event_dropdown,
        Container::Horizontal({event_confirm_btn, event_cancel_btn}),
    });

    // ==================== 主页面按钮 ====================
    auto query_by_event_btn = Button("按事件查询", [state, current_dialog, event_names] {
        state->LoadAllEvents();
        *event_names = state->GetEventNames();
        state->selected_event_index = 0;
        *current_dialog = QueryDialogType::EventQuery;
    });

    auto query_by_time_btn = Button("按时间查询", [state, current_dialog] {
        state->InitDateInputs();
        *current_dialog = QueryDialogType::TimeQuery;
    });
    
    auto show_all_btn = Button("显示全部", [state] {
        state->LoadAllBills();
    });

    auto return_btn = Button("返回", [] {
        Router::Instance().NavigateTo(Route::Home);
    });

    // 使用 Vertical 容器让上下键工作
    auto main_container = Container::Vertical({
        query_by_event_btn, 
        query_by_time_btn, 
        show_all_btn, 
        return_btn,
    }, button_index.get());

    // ==================== Tab 容器 ====================
    auto container = Container::Tab({
        main_container,
        time_dialog_container,
        event_dialog_container,
    }, tab_index.get());

    LayoutConfig config;
    config.button_width = 12;
    config.content_padding = 4;
    config.outer_padding = 2;
    config.content_height = 12;

    const int COL_ID = 8;
    const int COL_EVENT = 10;
    const int COL_AMOUNT = 10;
    const int COL_DESC = 12;
    const int COL_TIME = 18;

    // 事件处理：左右键翻页
    auto component_with_event = CatchEvent(container, [state, current_dialog](Event event) {
        // 弹窗打开时不处理翻页
        if (*current_dialog != QueryDialogType::None) {
            return false;
        }
        
        if (event == Event::ArrowLeft) {
            state->PrevPage();
            return true;
        }
        if (event == Event::ArrowRight) {
            state->NextPage();
            return true;
        }
        
        return false;
    });

    return Renderer(component_with_event, [=]() {
        // 更新 tab_index
        switch (*current_dialog) {
            case QueryDialogType::None:       *tab_index = 0; break;
            case QueryDialogType::TimeQuery:  *tab_index = 1; break;
            case QueryDialogType::EventQuery: *tab_index = 2; break;
        }
        
        auto user_info = RenderUserInfoBar(PageType::Profile);
        
        // 构建表格数据
        std::vector<std::vector<std::string>> table_data;
        table_data.push_back({"编号", "事项", "金额", "备注", "时间"});
        
        auto current_bills = state->GetCurrentPageBills();
        for (const auto& bill : current_bills) {
            table_data.push_back({
                std::to_string(bill.id),
                state->GetEventName(bill.event_id),
                state->FormatAmount(bill.amount),
                bill.description,
                state->FormatTime(bill.created_at)
            });
        }
        
        // 补充空行
        while (table_data.size() < static_cast<size_t>(state->page_size + 1)) {
            table_data.push_back({"", "", "", "", ""});
        }
        
        auto table = Table(table_data);
        
        table.SelectAll().SeparatorVertical(LIGHT);
        table.SelectAll().SeparatorHorizontal(LIGHT);
        table.SelectAll().Border(LIGHT);
        
        table.SelectColumn(0).DecorateCells(size(WIDTH, EQUAL, COL_ID));
        table.SelectColumn(1).DecorateCells(size(WIDTH, EQUAL, COL_EVENT));
        table.SelectColumn(2).DecorateCells(size(WIDTH, EQUAL, COL_AMOUNT));
        table.SelectColumn(3).DecorateCells(size(WIDTH, EQUAL, COL_DESC));
        table.SelectColumn(4).DecorateCells(size(WIDTH, EQUAL, COL_TIME));
        
        table.SelectAll().DecorateCells(center);
        table.SelectRow(0).Decorate(bold);
        table.SelectRow(0).DecorateCells(bgcolor(Color::Red));
        
        auto table_element = table.Render();

        auto page_info = hbox({
            text("第 " + std::to_string(state->current_page + 1) + 
                 "/" + std::to_string(state->GetTotalPages()) + " 页"),
            filler() | size(WIDTH, EQUAL, 15),
            text("← → 切换页码"),
        }) | center;

        auto content_area = vbox({
            table_element | center,
            text(""),
            page_info,
        });

        auto button_area = hbox({
            query_by_event_btn->Render() | size(WIDTH, EQUAL, config.button_width),
            text(" "),
            query_by_time_btn->Render() | size(WIDTH, EQUAL, config.button_width),
            filler(),
            show_all_btn->Render() | size(WIDTH, EQUAL, config.button_width),
            text(" "),
            return_btn->Render() | size(WIDTH, EQUAL, config.button_width),
        });
        
        auto main_page = CreatePageLayout(user_info, content_area, button_area, config);
        
        // ==================== 弹窗渲染 ====================
        if (*current_dialog == QueryDialogType::TimeQuery) {
            auto dialog_box = vbox({
                text("按时间查询") | bold | center,
                separator(),
                text(""),
                hbox({
                    text("查询日期: ") | vcenter,
                    year_input->Render() | size(WIDTH, EQUAL, 6) | border,
                    text("-") | vcenter,
                    month_input->Render() | size(WIDTH, EQUAL, 4) | border,
                    text("-") | vcenter,
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
        
        if (*current_dialog == QueryDialogType::EventQuery) {
            auto dialog_box = vbox({
                text("按事件查询") | bold | center,
                separator(),
                text(""),
                hbox({
                    text("选择事件: ") | vcenter,
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
        
        return main_page;
    });
}