#include "HomeScreen.h"
#include "App.h"
#include "Router.h"
#include "Session.h"
#include "UserInfoBar.h"
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

    const int BUTTON_WIDTH = 12;

    const int BUTTON_AREA_PADDING = 8;

    const int OUTER_PADDING = 2;
    
    // ==================== 渲染器 ====================
    return Renderer(container, [=, &session] {
        bool admin = session.IsAdmin();

        auto welcome_text = text("欢迎来到智能家庭手账系统") | bold | center;

        auto user_info = RenderUserInfoBar(PageType::Home);

        Element button_area;
        
        if (admin) {
            button_area = vbox({
                hbox({
                    user_manage_btn->Render() | size(WIDTH, EQUAL, BUTTON_WIDTH),
                    filler(),
                    event_manage_btn->Render() | size(WIDTH, EQUAL, BUTTON_WIDTH),
                }),
                filler(),
                hbox({
                    bill_manage_btn->Render() | size(WIDTH, EQUAL, BUTTON_WIDTH),
                    filler(),
                    admin_logout_btn->Render() | size(WIDTH, EQUAL, BUTTON_WIDTH),
                }),
            });
        } else {
            button_area = vbox({
                hbox({
                    password_change_btn->Render() | size(WIDTH, EQUAL, BUTTON_WIDTH),
                    filler(),
                    bill_create_btn->Render() | size(WIDTH, EQUAL, BUTTON_WIDTH),
                }),
                filler(),
                hbox({
                    user_stats_btn->Render() | size(WIDTH, EQUAL, BUTTON_WIDTH),
                    filler(),
                    user_logout_btn->Render() | size(WIDTH, EQUAL, BUTTON_WIDTH),
                }),
            });
        }

        auto button_area_with_padding = hbox({
            filler() | size(WIDTH, EQUAL, BUTTON_AREA_PADDING),
            button_area | flex,
            filler() | size(WIDTH, EQUAL, BUTTON_AREA_PADDING),
        }) | size(HEIGHT, EQUAL, 8);

        auto content = vbox({
            welcome_text,
            text(""),
            user_info,
            text(""),
            button_area_with_padding | flex,
            text(""),
        });

        return vbox({
            filler() | size(HEIGHT, EQUAL, 1),
            hbox({
                filler() | size(WIDTH, EQUAL, OUTER_PADDING),
                content | flex,
                filler() | size(WIDTH, EQUAL, OUTER_PADDING),
            }),
            filler() | size(HEIGHT, EQUAL, 1),
        }) | border;
    });
}