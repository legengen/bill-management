#include "RegisterScreen.h"
#include "App.h"
#include "Router.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <regex>

using namespace ftxui;

ftxui::Component CreateRegisterScreen() {
    // 状态变量
    auto phone = std::make_shared<std::string>();
    auto username = std::make_shared<std::string>();
    auto password = std::make_shared<std::string>();
    auto confirm_password = std::make_shared<std::string>();
    auto error_msg = std::make_shared<std::string>();
    auto success_msg = std::make_shared<std::string>();
    
    // 输入框配置
    InputOption phone_option;
    phone_option.multiline = false;
    
    InputOption password_option;
    password_option.password = true;
    password_option.multiline = false;
    
    // 创建输入组件
    auto phone_input = Input(phone. get(), "请输入11位手机号", phone_option);
    auto username_input = Input(username.get(), "请输入用户名");
    auto password_input = Input(password.get(), "请输入密码（至少6位）", password_option);
    auto confirm_input = Input(confirm_password.get(), "请再次输入密码", password_option);
    
    // 验证函数
    auto validate = [=]() -> bool {
        *error_msg = "";
        *success_msg = "";
        
        // 检查空值
        if (phone->empty()) {
            *error_msg = "手机号不能为空";
            return false;
        }
        
        if (username->empty()) {
            *error_msg = "用户名不能为空";
            return false;
        }
        
        if (password->empty()) {
            *error_msg = "密码不能为空";
            return false;
        }
        
        if (confirm_password->empty()) {
            *error_msg = "请确认密码";
            return false;
        }
        
        // 验证手机号格式（11位数字）
        if (phone->length() != 11) {
            *error_msg = "手机号必须是11位";
            return false;
        }
        
        // 检查手机号是否全是数字
        for (char c : *phone) {
            if (!std::isdigit(c)) {
                *error_msg = "手机号只能包含数字";
                return false;
            }
        }
        
        // 验证用户名长度
        if (username->length() < 2 || username->length() > 20) {
            *error_msg = "用户名长度需在2-20个字符之间";
            return false;
        }
        
        // 验证密码长度
        if (password->length() < 6) {
            *error_msg = "密码长度至少6位";
            return false;
        }
        
        if (password->length() > 32) {
            *error_msg = "密码长度不能超过32位";
            return false;
        }
        
        // 验证两次密码一致
        if (*password != *confirm_password) {
            *error_msg = "两次输入的密码不一致";
            return false;
        }
        
        return true;
    };
    
    // 注册按钮
    auto register_btn = Button("注 册", [=] {
        if (!validate()) {
            return;
        }
        
        try {
            auto& auth_service = App::Instance().GetAuthService();
            auto result = auth_service.Register(*phone, *username, *password);
            
            if (result. has_value()) {
                *success_msg = "注册成功！即将跳转到登录页面... ";
                *error_msg = "";
                
                // 清空输入
                phone->clear();
                username->clear();
                password->clear();
                confirm_password->clear();
                
                // 跳转到登录页面
                Router::Instance().NavigateTo(Route::Login);
            } else {
                *error_msg = "注册失败：该手机号已被注册";
                *success_msg = "";
            }
        } catch (const std::exception& e) {
            *error_msg = "系统错误：" + std::string(e.what());
            *success_msg = "";
        }
    });

    auto back_btn = Button("返 回", [] {
        Router::Instance().NavigateTo(Route::Visit);
    });

    auto container = Container::Vertical({
        phone_input,
        username_input,
        password_input,
        confirm_input,
        Container::Horizontal({
            register_btn,
            back_btn,
        }),
    });

    return Renderer(container, [=] {
        auto title = vbox({
            text("注册界面") | bold | center,
        });
        
        auto form = vbox({
            hbox({
                text("手 机 号: ") | size(WIDTH, EQUAL, 12),
                phone_input->Render() | flex | border,
            }),
            text(""),
            
            hbox({
                text("用 户 名: ") | size(WIDTH, EQUAL, 12),
                username_input->Render() | flex | border,
            }),
            text(""),
            
            hbox({
                text("密    码: ") | size(WIDTH, EQUAL, 12),
                password_input->Render() | flex | border,
            }),
            text(""),
            
            hbox({
                text("确认密码: ") | size(WIDTH, EQUAL, 12),
                confirm_input->Render() | flex | border,
            }),
        });
        
        auto messages = vbox({
            text(""),
            text(*error_msg) | color(Color::Red) | center,
            text(*success_msg) | color(Color::Green) | center,
            text(""),
        });
        
        auto buttons = hbox({
            filler(),
            register_btn->Render() | size(WIDTH, EQUAL, 12),
            text("  "),
            back_btn->Render() | size(WIDTH, EQUAL, 12),
            filler(),
        });
        
        auto content = vbox({
            title,
            text(""),
            form,
            messages,
            buttons,
            text(""),
        }) | border | size(WIDTH, EQUAL, 60) | center;
        
        return vbox({
            filler(),
            content,
            filler(),
        });
    });
}