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
    
    auto phone_input = Input(phone.get(), "手机号");
    InputOption password_option;
    password_option.password = true;
    auto password_input = Input(password.get(), "密码", password_option);
    
    auto login_button = Button("登 录", [=] {
        if (phone->empty() || password->empty()) {
            *error_msg = "手机号和密码不能为空";
            return;
        }
        
        auto& auth = App::Instance().GetAuthService();
        auto result = auth.Login(*phone, *password);
        
        if (result.has_value()) {
            Session::Instance().Login(*result);
            Router::Instance().NavigateTo(Route::Home);
        } else {
            *error_msg = "登录失败：手机号或密码错误";
        }
    });
    
    auto return_button = Button("返 回", [=] {
        Router::Instance().NavigateTo(Route::Visit);
    });
    
    auto container = Container::Vertical({
        phone_input,
        password_input,
        Container::Horizontal({
            login_button,
            return_button,
        }),
    });
    
    return Renderer(container, [=] {
        return vbox({
            filler(),
            vbox({
                filler() | size(HEIGHT, EQUAL, 2),
                text("登陆界面") | bold | center,
                filler() | size(HEIGHT, EQUAL, 2),
                hbox({text("账号:"), phone_input->Render()}) | size(WIDTH, EQUAL, 40),
                filler() | size(HEIGHT, EQUAL, 2),
                hbox({text("密码:"), password_input->Render()}) | size(WIDTH, EQUAL, 40),
                filler() | size(HEIGHT, EQUAL, 2),
                text(""),
                text(*error_msg) | color(Color::Red) | center,
                text(""),
                hbox({
                    login_button->Render() | size(WIDTH, EQUAL, 15),
                    text("  "),
                    return_button->Render() | size(WIDTH, EQUAL, 15),
                }) | center,
            }) | border | center,
            filler(),
        });
    });
}