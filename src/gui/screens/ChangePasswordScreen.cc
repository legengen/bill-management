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
        
        if (new_password->length() < 6) {
            DialogManager::Instance().ShowError("新密码长度至少6位");
            return false;
        }
        
        if (new_password->length() > 32) {
            DialogManager::Instance().ShowError("新密码长度不能超过32位");
            return false;
        }
        
        if (*new_password != *confirm_password) {
            DialogManager::Instance().ShowError("两次输入的新密码不一致");
            return false;
        }
        
        if (*old_password == *new_password) {
            DialogManager::Instance().ShowError("新密码不能与当前密码相同");
            return false;
        }
        
        return true;
    };
    
    // 提交按钮
    auto submit_btn = Button("确认修改", [=] {
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
                old_password->clear();
                new_password->clear();
                confirm_password->clear();
            } else {
                DialogManager::Instance().ShowError("修改失败：当前密码不正确");
            }
        } catch (const std::exception& e) {
            DialogManager::Instance().ShowError("系统错误：" + std::string(e.what()));
        }
    });
    
    // 返回按钮
    auto back_btn = Button("返回", [] {
        Router::Instance().NavigateTo(Route::Home);
    });
    
    // 组件容器
    auto container = Container::Vertical({
        old_pwd_input,
        new_pwd_input,
        confirm_pwd_input,
        Container::Horizontal({submit_btn, back_btn}),
    });
    
    // 布局常量
    const int BUTTON_WIDTH = 12;
    const int BUTTON_AREA_PADDING = 10;
    const int OUTER_PADDING = 2;
    const int INPUT_WIDTH = 15;
    const int LABEL_WIDTH = 10;
    
    // 渲染器
    return Renderer(container, [=] {
        auto& session = Session::Instance();
        std::string phone = session.GetCurrentUser()->phone;
        
        // 用户信息栏
        auto user_info = RenderUserInfoBar(PageType::Profile);
        
        // 四角布局表单区域
        auto form_area = vbox({
            hbox({
                hbox({
                    text("手机号:") | size(WIDTH, EQUAL, LABEL_WIDTH) | center,
                    text(phone) | bold | center,
                }),
                filler(),
                hbox({
                    text("当前密码:") | size(WIDTH, EQUAL, LABEL_WIDTH) | center,
                    old_pwd_input->Render() | size(WIDTH, EQUAL, INPUT_WIDTH) | border,
                }),
            }),
            filler(),
            hbox({
                hbox({
                    text("新密码:") | size(WIDTH, EQUAL, LABEL_WIDTH) | center,
                    new_pwd_input->Render() | size(WIDTH, EQUAL, INPUT_WIDTH) | border,
                }),
                filler(),
                hbox({
                    text("确认密码:") | size(WIDTH, EQUAL, LABEL_WIDTH) | center,
                    confirm_pwd_input->Render() | size(WIDTH, EQUAL, INPUT_WIDTH) | border,
                }),
            }),
        }) | size(HEIGHT, EQUAL, 6);

        auto form_with_padding = hbox({
            filler() | size(WIDTH, EQUAL, BUTTON_AREA_PADDING),
            form_area | flex,
            filler() | size(WIDTH, EQUAL, BUTTON_AREA_PADDING),
        });

        auto button_area = hbox({
            submit_btn->Render() | size(WIDTH, EQUAL, BUTTON_WIDTH),
            filler(),
            back_btn->Render() | size(WIDTH, EQUAL, BUTTON_WIDTH),
        });

        auto button_with_padding = hbox({
            filler() | size(WIDTH, EQUAL, BUTTON_AREA_PADDING),
            button_area | flex,
            filler() | size(WIDTH, EQUAL, BUTTON_AREA_PADDING),
        });

        auto content = vbox({
            text("欢迎来到智能家庭手账系统") | bold | center,
            text(""),
            user_info,
            text(""),
            text(""),
            form_with_padding,
            text(""),
            button_with_padding,
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