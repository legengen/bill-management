#include "BillManageScreen.h"
#include "App.h"
#include "Router.h"
#include "Session.h"
#include "UserInfoBar.h"
#include "PageLayout.h"
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
    
    auto container = Container::Horizontal({
        bill_query_btn,
        bill_stats_btn,
        return_btn,
    });

    LayoutConfig config;
    config.button_width = 14;
    config.content_padding = 4;
    config.outer_padding = 2;
    config.content_height = 12;
    
    return Renderer(container, [=] {
        auto user_info = RenderUserInfoBar();
        
        // 内容区域 - 四角分布
        auto content_area = vbox({
            // 上方两个按钮：左上和右上
            hbox({
                bill_query_btn->Render() | size(WIDTH, EQUAL, config.button_width),
                filler(),
                bill_stats_btn->Render() | size(WIDTH, EQUAL, config.button_width),
            }),
            filler(),
            // 下方两个位置：左下占位 和 右下返回
            hbox({
                text("") | size(WIDTH, EQUAL, config.button_width),  // 左下占位
                filler(),
                return_btn->Render() | size(WIDTH, EQUAL, config.button_width),  // 右下返回
            }),
        });
        
        // 底部按钮区域为空（按钮已在内容区四角分布）
        auto button_area = hbox({});
        
        return CreatePageLayout(user_info, content_area, button_area, config);
    });
}