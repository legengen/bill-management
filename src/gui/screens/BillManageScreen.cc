#include "BillManageScreen.h"
#include "App.h"
#include "Router.h"
#include "Session.h"
#include "UserInfoBar.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

ftxui::Component CreateBillManageScreen() {
    auto bill_query_btn = Button("账单查询", [] {
        Router::Instance().NavigateTo(Route::BillQuery);
    });
    
    auto bill_stats_btn = Button("账单统计", [] {
        Router::Instance().NavigateTo(Route::BillStats);
    });
    
    auto return_btn = Button("返回", [] {
        Router::Instance().NavigateTo(Route::Home);
    });
    
    auto container = Container::Vertical({
        bill_query_btn,
        bill_stats_btn,
        return_btn,
    });
    
    return Renderer(container, [=] {
        auto title = text("账单管理") | bold | center;
        auto user_info = RenderUserInfoBar(PageType::Home);
        
        auto buttons = vbox({
            bill_query_btn->Render() | size(WIDTH, EQUAL, 20) | center,
            text(""),
            bill_stats_btn->Render() | size(WIDTH, EQUAL, 20) | center,
            text(""),
            return_btn->Render() | size(WIDTH, EQUAL, 20) | center,
        }) | center;
        
        auto content = vbox({
            title,
            user_info,
            separator(),
            filler(),
            buttons,
            filler(),
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