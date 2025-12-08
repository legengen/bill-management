#include "Dialog.h"
#include "ftxui/component/event.hpp"

using namespace ftxui;

void DialogManager::Show(const DialogConfig& config) {
    config_ = config;
    visible_ = true;
}

void DialogManager::ShowInfo(const std::string& message, const std::string& title) {
    DialogConfig config;
    config.type = DialogType::Info;
    config.title = title;
    config.message = message;
    Show(config);
}

void DialogManager::ShowSuccess(const std::string& message, const std::string& title) {
    DialogConfig config;
    config.type = DialogType::Success;
    config.title = title;
    config.message = message;
    Show(config);
}

void DialogManager::ShowWarning(const std::string& message, const std::string& title) {
    DialogConfig config;
    config.type = DialogType::Warning;
    config.title = title;
    config.message = message;
    Show(config);
}

void DialogManager::ShowError(const std::string& message, const std::string& title) {
    DialogConfig config;
    config.type = DialogType::Error;
    config.title = title;
    config.message = message;
    Show(config);
}

void DialogManager::ShowConfirm(const std::string& message,
                                 std::function<void()> on_confirm,
                                 std::function<void()> on_cancel,
                                 const std::string& title) {
    DialogConfig config;
    config. type = DialogType::Confirm;
    config.title = title;
    config.message = message;
    config.on_confirm = on_confirm;
    config. on_cancel = on_cancel;
    Show(config);
}

void DialogManager::Close() {
    visible_ = false;
    config_ = DialogConfig{};
}

void DialogManager::HandleConfirm() {
    if (config_.on_confirm) {
        config_. on_confirm();
    }
    Close();
}

void DialogManager::HandleCancel() {
    if (config_.on_cancel) {
        config_.on_cancel();
    }
    Close();
}

ftxui::Element RenderDialog() {
    auto& manager = DialogManager::Instance();
    
    if (!manager.IsVisible()) {
        return text("");
    }
    
    const auto& config = manager.GetConfig();
    
    // 标题前缀
    std::string prefix;
    switch (config.type) {
        case DialogType::Info:    prefix = "[i] "; break;
        case DialogType::Success: prefix = "[v] "; break;
        case DialogType::Warning: prefix = "[! ] "; break;
        case DialogType::Error:   prefix = "[x] "; break;
        case DialogType::Confirm: prefix = "[? ] "; break;
    }
    
    // 消息内容
    Elements message_lines;
    std::string line;
    std::istringstream iss(config.message);
    while (std::getline(iss, line)) {
        message_lines.push_back(text(line) | center);
    }
    
    // 对话框内容
    auto dialog_content = vbox({
        text(prefix + config.title) | bold | center,
        separator(),
        text(""),
        vbox(message_lines),
        text(""),
    });
    
    // 按钮提示
    Element button_hint;
    if (config.type == DialogType::Confirm) {
        button_hint = hbox({
            text("[Enter] " + config.confirm_text),
            text("    "),
            text("[Esc] " + config.cancel_text),
        }) | center;
    } else {
        button_hint = text("[Enter] " + config.confirm_text) | center;
    }
    
    auto dialog_box = vbox({
        dialog_content,
        separator(),
        button_hint,
    }) | border | size(WIDTH, GREATER_THAN, 30) | size(WIDTH, LESS_THAN, 50);
    
    // 居中显示
    return vbox({
        filler(),
        hbox({
            filler(),
            dialog_box | clear_under,
            filler(),
        }),
        filler(),
    });
}

ftxui::Component CreateDialogComponent() {
    auto& manager = DialogManager::Instance();
    
    auto component = Renderer([] {
        return RenderDialog();
    });
    
    return CatchEvent(component, [&manager](Event event) {
        if (! manager.IsVisible()) {
            return false;
        }
        
        if (event == Event::Return) {
            manager.HandleConfirm();
            return true;
        }
        
        if (event == Event::Escape) {
            if (manager.GetConfig().type == DialogType::Confirm) {
                manager.HandleCancel();
            } else {
                manager.HandleConfirm();
            }
            return true;
        }
        
        // 提示框显示时拦截其他事件
        return true;
    });
}

ftxui::Component WithDialog(ftxui::Component inner) {
    auto dialog = CreateDialogComponent();
    
    return Renderer(inner, [inner, dialog] {
        auto& manager = DialogManager::Instance();
        
        auto base = inner->Render();
        
        if (manager.IsVisible()) {
            // 叠加显示提示框
            return dbox({
                base | dim,  // 背景变暗
                RenderDialog(),
            });
        }
        
        return base;
    }) | CatchEvent([inner](Event event) {
        auto& manager = DialogManager::Instance();
        
        if (manager.IsVisible()) {
            if (event == Event::Return) {
                manager.HandleConfirm();
                return true;
            }
            
            if (event == Event::Escape) {
                if (manager.GetConfig().type == DialogType::Confirm) {
                    manager.HandleCancel();
                } else {
                    manager.HandleConfirm();
                }
                return true;
            }
            
            // 拦截所有事件
            return true;
        }
        
        return inner->OnEvent(event);
    });
}