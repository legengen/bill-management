#include <memory>
#include <string>

#include "ftxui/component/captured_mouse.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/component/component_base.hpp"
#include "ftxui/dom/elements.hpp"
#include "VisitScreen.h"
#include "Router.h"

using namespace ftxui;

ftxui::Component CreateVisitScreen() {
    
    auto login_btn = Button("登录", [] {
        Router::Instance().NavigateTo(Route::Login);
    });
    
    auto register_btn = Button("注册", [] {
        Router::Instance().NavigateTo(Route::Register);
    });
    
    auto quit_btn = Button("退出", [] {
        // TODO: 发送退出事件或设置退出标志
    });

    auto buttons = Container::Vertical({
        login_btn,
        register_btn,
        quit_btn,
    });

    return Renderer(buttons, [=] {
        auto content = vbox({
            filler() | size(HEIGHT, EQUAL, 2),
            text("智能家庭手账系统") | bold | center,
            filler() | size(HEIGHT, EQUAL, 2),
            vbox({
                login_btn->Render(),
                filler() | size(HEIGHT, EQUAL, 2),
                register_btn->Render(),
                filler() | size(HEIGHT, EQUAL, 2),
                quit_btn->Render(),
                filler() | size(HEIGHT, EQUAL, 2),
            }),
        });
        
        return hbox({
            filler(),
            content | flex,
            filler(),
        })
            | size(WIDTH, GREATER_THAN, 30)
            | size(HEIGHT, GREATER_THAN, 12)
            | border;
    });
}