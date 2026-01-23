#include "ProfileScreen.h"
#include "App.h"
#include "Router.h"
#include "Session.h"
#include "UserInfoBar.h"
#include "Dialog.h"
#include <ftxui/component/component.hpp>
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
    int page_size = 5;
    
    // 筛选状态
    bool is_filtered = false;
    std::string filter_info;
    
    // 日期输入（单个日期）
    std::string query_year;
    std::string query_month;
    std::string query_day;
    
    void LoadAllBills() {
        auto& session = Session::Instance();
        auto& bill_service = App::Instance().GetBillService();
        bills = bill_service.queryByPhone(session.GetPhone());
        is_filtered = false;
        filter_info.clear();
        current_page = 0;
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
    
    // 查询指定日期的账单
    bool QueryByDate() {
        model::Timestamp start_ts;
        
        if (!ParseDate(query_year, query_month, query_day, start_ts)) {
            return false;
        }
        
        // 当天结束时间 = 开始时间 + 24小时
        model::Timestamp end_ts = start_ts + 24 * 60 * 60;
        
        // 直接调用 BillService 查询
        auto& session = Session::Instance();
        auto& bill_service = App::Instance().GetBillService();
        bills = bill_service.QueryByTime(session.GetUserId(), start_ts, end_ts);
        
        is_filtered = true;
        filter_info = query_year + "-" + query_month + "-" + query_day;
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

ftxui::Component CreateProfileScreen() {
    auto state = std::make_shared<ProfileState>();
    state->LoadAllBills();
    state->InitDateInputs();
    
    auto show_time_dialog = std::make_shared<bool>(false);

    // 单个日期输入
    auto year_input = Input(&state->query_year, "年");
    auto month_input = Input(&state->query_month, "月");
    auto day_input = Input(&state->query_day, "日");

    auto dialog_confirm_btn = Button("确认", [state, show_time_dialog] {
        if (state->QueryByDate()) {
            *show_time_dialog = false;
            DialogManager::Instance().ShowSuccess(
                "查询完成，共 " + std::to_string(state->bills.size()) + " 条记录"
            );
        } else {
            DialogManager::Instance().ShowError("日期格式错误");
        }
    });

    auto dialog_cancel_btn = Button("取消", [show_time_dialog] {
        *show_time_dialog = false;
    });

    auto dialog_container = Container::Vertical({
        Container::Horizontal({year_input, month_input, day_input}),
        Container::Horizontal({dialog_confirm_btn, dialog_cancel_btn}),
    });

    auto query_by_event_btn = Button("按事件查询", [] {
        Router::Instance().NavigateTo(Route::QueryByEvent);
    });

    auto query_by_time_btn = Button("按时间查询", [state, show_time_dialog] {
        state->InitDateInputs();
        *show_time_dialog = true;
    });
    
    auto show_all_btn = Button("显示全部", [state] {
        state->LoadAllBills();
    });

    auto return_btn = Button("返回", [] {
        Router::Instance().NavigateTo(Route::Home);
    });

    auto prev_btn = Button("◀ 上一页", [state] {
        state->PrevPage();
    });

    auto next_btn = Button("下一页 ▶", [state] {
        state->NextPage();
    });

    auto main_container = Container::Vertical({
        Container::Horizontal({prev_btn, next_btn}),
        Container::Horizontal({query_by_event_btn, query_by_time_btn, show_all_btn, return_btn}),
    });

    auto tab_index = std::make_shared<int>(0);
    auto container = Container::Tab({
        main_container,
        dialog_container,
    }, tab_index.get());

    return Renderer(container, [=] {
        *tab_index = *show_time_dialog ? 1 : 0;
        
        auto title = text("账单查询") | bold | center;
        auto user_info = RenderUserInfoBar(PageType::Profile);
        
        Element filter_status;
        if (state->is_filtered) {
            filter_status = hbox({
                text("查询日期: ") | bold,
                text(state->filter_info) | color(Color::Green),
                text("  共 " + std::to_string(state->bills.size()) + " 条记录"),
            }) | center;
        } else {
            filter_status = text("显示全部账单，共 " + std::to_string(state->bills.size()) + " 条记录") | center | dim;
        }
        
        std::vector<std::vector<std::string>> table_data;
        table_data.push_back({"账单编号", "事项名称", "金额", "备注", "时间"});
        
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
        
        if (current_bills.empty()) {
            table_data.push_back({"", "", "暂无账单数据", "", ""});
        }
        
        auto table = Table(table_data);
        table.SelectAll().Border(LIGHT);
        table.SelectRow(0).Decorate(bold);
        table.SelectColumn(0).DecorateCells(size(WIDTH, EQUAL, 10));
        table.SelectColumn(1).DecorateCells(size(WIDTH, EQUAL, 12));
        table.SelectColumn(2).DecorateCells(size(WIDTH, EQUAL, 12));
        table.SelectColumn(3).DecorateCells(size(WIDTH, EQUAL, 15));
        table.SelectColumn(4).DecorateCells(size(WIDTH, EQUAL, 18));
        table.SelectAll().DecorateCells(center);
        
        auto table_element = table.Render();
        
        auto page_info = text("第 " + std::to_string(state->current_page + 1) + 
                              " / " + std::to_string(state->GetTotalPages()) + " 页") | center;
        
        auto page_buttons = hbox({
            filler(),
            prev_btn->Render() | size(WIDTH, EQUAL, 12),
            text("  "),
            next_btn->Render() | size(WIDTH, EQUAL, 12),
            filler(),
        });
        
        auto bottom_buttons = hbox({
            filler(),
            query_by_event_btn->Render() | size(WIDTH, EQUAL, 14),
            text(" "),
            query_by_time_btn->Render() | size(WIDTH, EQUAL, 14),
            text(" "),
            show_all_btn->Render() | size(WIDTH, EQUAL, 12),
            text(" "),
            return_btn->Render() | size(WIDTH, EQUAL, 10),
            filler(),
        });
        
        auto content = vbox({
            title,
            user_info,
            filter_status,
            separator(),
            table_element | center,
            text(""),
            page_info,
            page_buttons,
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
        
        if (*show_time_dialog) {
            // 用空格填充创建不透明背景
            auto make_solid_background = [](Element content) {
                return content | bgcolor(Color::Default);
            };
            
            auto dialog_box = vbox({
                text("按时间查询") | bold | center,
                separator(),
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
                    dialog_confirm_btn->Render() | size(WIDTH, EQUAL, 10),
                    text("  "),
                    dialog_cancel_btn->Render() | size(WIDTH, EQUAL, 10),
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