#include "LoginScreen.h"
#include "App.h"
#include "Router.h"
#include "Session.h"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

Component CreateLoginScreen() {
    auto phone = std::make_shared<std::string>();
    auto password = std::make_shared<std::string>();
    auto error_msg = std::make_shared<std::string>();
    
    auto phone_input = Input(phone. get(), "手机号");
    InputOption password_option;
    password_option.password = true;
    auto password_input = Input(password.get(), "密码", password_option);
    
    auto login_button = Button("登 录", [=] {
        auto& auth = App::Instance().GetAuthService();
        auto result = auth.Login(*phone, *password);
        
        if (result.has_value()) {
            Session::Instance().Login(*result);
            Router::Instance().NavigateTo(Route::Home);
        } else {
            *error_msg = "登录失败：手机号或密码错误";
        }
    });
    
    auto register_button = Button("注 册", [=] {
        Router::Instance().NavigateTo(Route::Register);
    });
    
    auto container = Container::Vertical({
        phone_input,
        password_input,
        Container::Horizontal({
            login_button,
            register_button,
        }),
    });
    
    return Renderer(container, [=] {
        return vbox({
            filler(),
            vbox({
                text("📊 账单管理系统") | bold | center,
                separator(),
                text(""),
                hbox({text("手机号: "), phone_input->Render()}) | size(WIDTH, EQUAL, 40),
                hbox({text("密  码: "), password_input->Render()}) | size(WIDTH, EQUAL, 40),
                text(""),
                text(*error_msg) | color(Color::Red) | center,
                text(""),
                hbox({
                    login_button->Render() | size(WIDTH, EQUAL, 15),
                    text("  "),
                    register_button->Render() | size(WIDTH, EQUAL, 15),
                }) | center,
            }) | border | center,
            filler(),
        });
    });
}