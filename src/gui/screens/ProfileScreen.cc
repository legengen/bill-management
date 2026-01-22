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
    
    void LoadBills() {
        auto& session = Session::Instance();
        auto& bill_service = App::Instance().GetBillService();
        bills = bill_service.queryByPhone(session.GetPhone());
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

ftxui::Component CreateProfileScreen() {
    auto state = std::make_shared<ProfileState>();
    state->LoadBills();

    auto query_by_event_btn = Button("按事件查询", [state] {
        DialogManager::Instance().ShowInfo("按事件查询功能开发中...");
    });

    auto query_by_time_btn = Button("按时间查询", [state] {
        DialogManager::Instance().ShowInfo("按时间查询功能开发中...");
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

    auto container = Container::Vertical({
        Container::Horizontal({prev_btn, next_btn}),
        Container::Horizontal({query_by_event_btn, query_by_time_btn, return_btn}),
    });

    return Renderer(container, [=] {
        auto title = text("账单查询") | bold | center;  // 修改标题
        auto user_info = RenderUserInfoBar(PageType::Profile);
        
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
            text("  "),
            query_by_time_btn->Render() | size(WIDTH, EQUAL, 14),
            text("  "),
            return_btn->Render() | size(WIDTH, EQUAL, 10),
            filler(),
        });
        
        auto content = vbox({
            title,
            user_info,
            table_element | center,
            text(""),
            page_info,
            page_buttons,
            text(""),
            bottom_buttons,
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