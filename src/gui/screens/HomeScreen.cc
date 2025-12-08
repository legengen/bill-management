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
    auto user_manage_btn = Button(" 👥 用户管理 ", [] {
        Router::Instance().NavigateTo(Route::UserManage);
    });
    
    auto event_manage_btn = Button(" ⚙️ 事件管理 ", [] {
        Router::Instance().NavigateTo(Route::EventManage);
    });
    
    auto bill_manage_btn = Button(" 📋 账单管理 ", [] {
        Router::Instance().NavigateTo(Route::BillList);
    });
    
    auto admin_logout_btn = Button(" 🚪 退出登录 ", [] {
        Session::Instance().Logout();
        Router::Instance().NavigateTo(Route::Visit);
    });
    
    // ==================== 普通用户按钮 ====================
    auto password_change_btn = Button("修改密码 ", [] {
        Router::Instance().NavigateTo(Route::ChangePassword);
    });

    auto bill_create_btn = Button("新建手账", [] {
        Router::Instance().NavigateTo(Route::BillCreate);
    });
    
    auto user_stats_btn = Button("账单查询", [] {
        Router::Instance().NavigateTo(Route::Profile);
    });
    
    auto user_logout_btn = Button("退出", [] {
        Session::Instance().Logout();
        Router::Instance().Exit();
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
    
    // ==================== 渲染器 ====================
    return Renderer(container, [=, &session] {
        bool admin = session.IsAdmin();
        std::string username = session.GetUsername();
        
        // 欢迎信息
        auto welcome_text = vbox({
            text("欢迎来到智能家庭手账系统") | bold | center,
        });
        
        // 功能按钮区域
        Element button_area;
        
        if (admin) {
            // 管理员布局：四角分布
            button_area = vbox({
                // 上半部分
                hbox({
                    user_manage_btn->Render() | flex,
                    filler() | size(WIDTH, EQUAL, 10),
                    event_manage_btn->Render() | flex,
                }),
                filler() | size(HEIGHT, EQUAL, 3),
                // 提示信息
                hbox({

                }),
                filler() | size(HEIGHT, EQUAL, 3),
                // 下半部分
                hbox({
                    bill_manage_btn->Render() | flex,
                    filler() | size(WIDTH, EQUAL, 10),
                    admin_logout_btn->Render() | flex,
                }),
            });
        } else {
            // 普通用户布局：四角分布
            button_area = vbox({
                // 上半部分
                hbox({
                    password_change_btn->Render() | flex,
                    filler() | size(WIDTH, EQUAL, 10),
                    bill_create_btn->Render() | flex,
                }),
                filler() | size(HEIGHT, EQUAL, 3),
                // 提示信息
                hbox({
                    
                }),
                filler() | size(HEIGHT, EQUAL, 3),
                // 下半部分
                hbox({
                    user_stats_btn->Render() | flex,
                    filler() | size(WIDTH, EQUAL, 10),
                    user_logout_btn->Render() | flex,
                }),
            });
        }
        
        // 最终布局：外层留白
        auto content = vbox({ 
            // 当前页面信息
            welcome_text,
            text(""),

            // 上半部：用户信息栏
            RenderUserInfoBar(PageType::Home),
            text(""),
            
            // 按钮区域
            button_area | flex,
            text(""),
        });
        
        // 外层 vbox + hbox 留白
        return vbox({
            filler() | size(HEIGHT, EQUAL, 1),  // 上边距
            hbox({
                filler() | size(WIDTH, EQUAL, 2),  // 左边距
                content | flex,
                filler() | size(WIDTH, EQUAL, 2),  // 右边距
            }),
            filler() | size(HEIGHT, EQUAL, 1),  // 下边距
        }) | border;
    });
}