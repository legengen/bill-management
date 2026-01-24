#include "HomeScreen.h"
#include "App.h"
#include "Router.h"
#include "Session.h"
#include "UserInfoBar.h"
#include "PageLayout.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

ftxui::Component CreateHomeScreen() {
    auto& session = Session::Instance();
    bool is_admin = session.IsAdmin();
    
    // ==================== 管理员按钮 ====================
    auto user_manage_btn = Button("用户管理", [] {
        Router::Instance().NavigateTo(Route::UserManage);
    });
    
    auto event_manage_btn = Button("事件管理", [] {
        Router::Instance().NavigateTo(Route::EventManage);
    });
    
    auto bill_manage_btn = Button("账单管理", [] {
        Router::Instance().NavigateTo(Route::BillManage);
    });
    
    auto admin_logout_btn = Button("退出登录", [] {
        Session::Instance().Logout();
        Router::Instance().NavigateTo(Route::Visit);
    });
    
    // ==================== 普通用户按钮 ====================
    auto password_change_btn = Button("修改密码", [] {
        Router::Instance().NavigateTo(Route::ChangePassword);
    });

    auto bill_create_btn = Button("新建手账", [] {
        Router::Instance().NavigateTo(Route::BillCreate);
    });
    
    auto user_stats_btn = Button("账单查询", [] {
        Router::Instance().NavigateTo(Route::Profile);
    });
    
    auto user_logout_btn = Button("退出登录", [] {
        Session::Instance().Logout();
        Router::Instance().NavigateTo(Route::Visit);
    });
    
    // ==================== 组件容器 ====================
    Component container;
    
    if (is_admin) {
        container = Container::Vertical({
            Container::Horizontal({user_manage_btn, event_manage_btn}),
            Container::Horizontal({bill_manage_btn, admin_logout_btn}),
        });
    } else {
        container = Container::Vertical({
            Container::Horizontal({password_change_btn, bill_create_btn}),
            Container::Horizontal({user_stats_btn, user_logout_btn}),
        });
    }

    LayoutConfig config;
    config.button_width = 12;
    config.content_padding = 8;
    config.outer_padding = 2;
    config.content_height = 8;
    
    return Renderer(container, [=, &session] {
        bool admin = session.IsAdmin();
        
        auto user_info = RenderUserInfoBar(PageType::Home);
        
        // 内容区域：四角分布的按钮
        Element content_area;
        if (admin) {
            content_area = vbox({
                hbox({
                    user_manage_btn->Render() | size(WIDTH, EQUAL, config.button_width),
                    filler(),
                    event_manage_btn->Render() | size(WIDTH, EQUAL, config.button_width),
                }),
                filler(),
                hbox({
                    bill_manage_btn->Render() | size(WIDTH, EQUAL, config.button_width),
                    filler(),
                    admin_logout_btn->Render() | size(WIDTH, EQUAL, config.button_width),
                }),
            });
        } else {
            content_area = vbox({
                hbox({
                    password_change_btn->Render() | size(WIDTH, EQUAL, config.button_width),
                    filler(),
                    bill_create_btn->Render() | size(WIDTH, EQUAL, config.button_width),
                }),
                filler(),
                hbox({
                    user_stats_btn->Render() | size(WIDTH, EQUAL, config.button_width),
                    filler(),
                    user_logout_btn->Render() | size(WIDTH, EQUAL, config.button_width),
                }),
            });
        }
        
        // Home 页面没有底部按钮，使用空元素
        Element button_area = text("");
        
        return CreatePageLayout(user_info, content_area, button_area, config);
    });
}