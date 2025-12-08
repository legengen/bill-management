#include "ChangePasswordScreen.h"
#include "App.h"
#include "Router.h"
#include "Session.h"
#include "UserInfoBar.h"
#include "Dialog.h" 
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

ftxui::Component CreateChangePasswordScreen() {
    // 状态变量
    auto old_password = std::make_shared<std::string>();
    auto new_password = std::make_shared<std::string>();
    auto confirm_password = std::make_shared<std::string>();
    auto error_msg = std::make_shared<std::string>();
    auto success_msg = std::make_shared<std::string>();
    
    // 输入框配置
    InputOption password_option;
    password_option.password = true;
    password_option.multiline = false;
    
    // 创建输入组件
    auto old_pwd_input = Input(old_password.get(), password_option);
    auto new_pwd_input = Input(new_password.get(), password_option);
    auto confirm_pwd_input = Input(confirm_password.get(), password_option);
    
    // 验证函数
    auto validate = [=]() -> bool {
        *error_msg = "";
        *success_msg = "";
        
        // 检查空值
        if (old_password->empty()) {
            DialogManager::Instance().ShowError("请输入当前密码");
            return false;
        }
        
        if (new_password->empty()) {
            DialogManager::Instance().ShowError("请输入新密码");
            return false;
        }
        
        if (confirm_password->empty()) {
            DialogManager::Instance().ShowError("请确认新密码");
            return false;
        }
        
        // 验证新密码长度
        if (new_password->length() < 6) {
            DialogManager::Instance().ShowError("新密码长度至少6位");
            return false;
        }
        
        if (new_password->length() > 32) {
            DialogManager::Instance().ShowError("新密码长度不能超过32位");
            return false;
        }
        
        // 验证两次新密码一致
        if (*new_password != *confirm_password) {
            DialogManager::Instance().ShowError("两次输入的新密码不一致");
            return false;
        }
        
        // 验证新旧密码不同
        if (*old_password == *new_password) {
            DialogManager::Instance().ShowError("新密码不能与当前密码相同");
            return false;
        }
        
        return true;
    };

    auto welcome_text = vbox({
            text("欢迎来到智能家庭手账系统") | bold | center,
    });
    
    // 提交按钮
    auto submit_btn = Button("确认修改 ", [=] {
        if (!validate()) {
            return;
        }
        
        try {
            auto& session = Session::Instance();
            auto& auth_service = App::Instance().GetAuthService();
            
            int user_id = session.GetUserId();
            
            bool result = auth_service.ResetPassword(user_id, *old_password, *new_password);
            
            if (result) {
                DialogManager::Instance().ShowSuccess("密码修改成功！");
                
                // 清空输入
                old_password->clear();
                new_password->clear();
                confirm_password->clear();
                
                // 可选：延迟返回或提示重新登录
            } else {
                DialogManager::Instance().ShowError("修改失败：当前密码不正确");
            }
        } catch (const std::exception& e) {
            DialogManager::Instance().ShowError("系统错误：" + std::string(e.what()));
        }
    });
    
    // 返回按钮
    auto back_btn = Button(" 返回 ", [] {
        Router::Instance().NavigateTo(Route::Home);
    });
    
    // 组件容器
    auto container = Container::Vertical({
        old_pwd_input,
        new_pwd_input,
        confirm_pwd_input,
        Container::Horizontal({
            submit_btn,
            back_btn,
        }),
    });
    
    // 渲染器
    auto renderer = Renderer(container, [=] {
        // 获取用户手机号
        auto& session = Session::Instance();
        std::string phone = session.GetCurrentUser()->phone;
        
        // 标题
        auto title = vbox({
            text("修改密码") | bold | center,
            separator(),
        });
        
        // 按钮区域
        auto buttons = hbox({
            filler(),
            submit_btn->Render() | size(WIDTH, EQUAL, 14),
            text("  "),
            back_btn->Render() | size(WIDTH, EQUAL, 10),
            filler(),
        });
        
        // 四角布局：左上手机号，右上原密码，左下新密码，右下确认密码
        auto form = vbox({
            hbox({
                // 左上：手机号
                hbox({
                    text("手机号: " + phone) | bold | center,
                    filler(),
                }) | flex,
                // 右上：原密码输入框
                hbox({
                    text("当前密码:") | dim | center,
                    old_pwd_input->Render() | border | size(WIDTH, EQUAL, 25) | center,
                }) | flex,
            }),
            text(""),
            hbox({
                // 左下：新密码输入框
                hbox({
                    text("新密码:") | dim | center,
                    new_pwd_input->Render() | border | size(WIDTH, EQUAL, 25) | center,
                }) | flex,
                // 右下：确认新密码框
                hbox({
                    text("确认密码:") | dim | center,
                    confirm_pwd_input->Render() | border | size(WIDTH, EQUAL, 25) | center,
                }) | flex,
            }),
        });
        
        // 表单容器（下半部分）
        auto form_container = vbox({
            form,
            buttons,
        }) | size(WIDTH, GREATER_THAN, 55);
        
        // 内容区域
        auto content = vbox({
            // 用户信息栏
            welcome_text,
            RenderUserInfoBar(PageType::Profile),
            text(""),
            // 表单居中
            hbox({
                filler(),
                form_container,
                filler(),
            }),
        });
        
        // 外层留白
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

    return WithDialog(renderer);
}