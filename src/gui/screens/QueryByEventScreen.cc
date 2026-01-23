#include "QueryByEventScreen.h"
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

struct QueryByEventState {
    // 事件选择
    std::vector<std::string> event_names;
    std::vector<int> event_ids;
    int event_selected = 0;
    
    // 查询结果
    std::vector<model::Bill> bills;
    int current_page = 0;
    int page_size = 5;
    bool has_queried = false;  // 是否已查询
    
    void LoadEvents() {
        auto& event_service = App::Instance().GetEventService();
        auto events = event_service.GetAllEvents();
        
        event_names.clear();
        event_ids.clear();
        
        for (const auto& event : events) {
            if (event.status == model::EventStatus::Available) {
                event_names.push_back(event.name);
                event_ids.push_back(event.id);
            }
        }
        
        if (event_names.empty()) {
            event_names.push_back("(无可用事件)");
            event_ids.push_back(-1);
        }
    }
    
    int GetSelectedEventId() const {
        if (event_selected >= 0 && event_selected < static_cast<int>(event_ids.size())) {
            return event_ids[event_selected];
        }
        return -1;
    }
    
    std::string GetSelectedEventName() const {
        if (event_selected >= 0 && event_selected < static_cast<int>(event_names.size())) {
            return event_names[event_selected];
        }
        return "";
    }
    
    void Query() {
        int event_id = GetSelectedEventId();
        if (event_id < 0) {
            bills.clear();
            return;
        }
        
        auto& session = Session::Instance();
        auto& bill_service = App::Instance().GetBillService();
        bills = bill_service.queryByEvent(session.GetUserId(), event_id);
        current_page = 0;
        has_queried = true;
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
};

ftxui::Component CreateQueryByEventScreen() {
    auto state = std::make_shared<QueryByEventState>();
    state->LoadEvents();

    // 事件下拉框
    auto event_dropdown = Dropdown(&state->event_names, &state->event_selected);

    // 查询按钮
    auto query_btn = Button("查询", [state] {
        if (state->GetSelectedEventId() < 0) {
            DialogManager::Instance().ShowError("请选择有效的事件类型");
            return;
        }
        state->Query();
    });

    // 返回按钮
    auto return_btn = Button("返回", [] {
        Router::Instance().NavigateTo(Route::Profile);
    });

    // 上一页按钮
    auto prev_btn = Button("◀ 上一页", [state] {
        state->PrevPage();
    });

    // 下一页按钮
    auto next_btn = Button("下一页 ▶", [state] {
        state->NextPage();
    });

    auto container = Container::Vertical({
        event_dropdown,
        Container::Horizontal({query_btn, return_btn}),
        Container::Horizontal({prev_btn, next_btn}),
    });

    return Renderer(container, [=] {
        auto title = text("按事件查询") | bold | center;
        auto user_info = RenderUserInfoBar(PageType::Profile);
        
        // 查询条件区域
        auto query_area = hbox({
            text("选择事件: ") | center,
            event_dropdown->Render() | size(WIDTH, EQUAL, 20),
            text("  "),
            query_btn->Render() | size(WIDTH, EQUAL, 10),
        }) | center;
        
        // 构建表格
        std::vector<std::vector<std::string>> table_data;
        table_data.push_back({"账单编号", "事项名称", "金额", "备注", "时间"});
        
        Element table_element;
        Element page_info_element;
        Element page_buttons_element;
        
        if (state->has_queried) {
            auto current_bills = state->GetCurrentPageBills();
            
            if (current_bills.empty()) {
                table_data.push_back({"", "", "该事件暂无账单", "", ""});
            } else {
                for (const auto& bill : current_bills) {
                    table_data.push_back({
                        std::to_string(bill.id),
                        state->GetEventName(bill.event_id),
                        state->FormatAmount(bill.amount),
                        bill.description,
                        state->FormatTime(bill.created_at)
                    });
                }
            }
            
            auto table = Table(table_data);
            table.SelectAll().Border(LIGHT);
            table.SelectRow(0).Decorate(bold);
            table.SelectRow(0).BorderBottom(LIGHT);
            table.SelectColumn(0).DecorateCells(size(WIDTH, EQUAL, 10));
            table.SelectColumn(1).DecorateCells(size(WIDTH, EQUAL, 12));
            table.SelectColumn(2).DecorateCells(size(WIDTH, EQUAL, 12));
            table.SelectColumn(3).DecorateCells(size(WIDTH, EQUAL, 15));
            table.SelectColumn(4).DecorateCells(size(WIDTH, EQUAL, 18));
            table.SelectAll().DecorateCells(center);
            
            table_element = table.Render() | center;
            
            page_info_element = text("第 " + std::to_string(state->current_page + 1) + 
                                     " / " + std::to_string(state->GetTotalPages()) + " 页  " +
                                     "共 " + std::to_string(state->bills.size()) + " 条记录") | center;
            
            page_buttons_element = hbox({
                filler(),
                prev_btn->Render() | size(WIDTH, EQUAL, 12),
                text("  "),
                next_btn->Render() | size(WIDTH, EQUAL, 12),
                filler(),
            });
        } else {
            table_element = text("请选择事件后点击查询") | center | dim;
            page_info_element = text("");
            page_buttons_element = text("");
        }
        
        auto bottom_buttons = hbox({
            filler(),
            return_btn->Render() | size(WIDTH, EQUAL, 10),
            filler(),
        });
        
        auto content = vbox({
            title,
            user_info,
            text(""),
            query_area,
            bottom_buttons,
            text(""),
            table_element,
            text(""),
            page_info_element,
            page_buttons_element,
            text(""),
        });
        
        return vbox({
            filler() | size(HEIGHT, EQUAL, 1),
            hbox({
                filler() | size(WIDTH, EQUAL, 2),
                content | flex,
                filler() | size(WIDTH, EQUAL, 2),
            }),
            filler() | size(HEIGHT, EQUAL, 1),
        }) | border;
    });
}