#pragma once
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <string>
#include <functional>

// 提示框类型
enum class DialogType {
    Info,       // 信息提示
    Success,    // 成功提示
    Warning,    // 警告提示
    Error,      // 错误提示
    Confirm     // 确认对话框
};

// 提示框配置
struct DialogConfig {
    std::string title = "提示";
    std::string message;
    DialogType type = DialogType::Info;
    std::string confirm_text = "确定";
    std::string cancel_text = "取消";
    std::function<void()> on_confirm = nullptr;
    std::function<void()> on_cancel = nullptr;
};

// 提示框状态管理
class DialogManager {
public:
    static DialogManager& Instance() {
        static DialogManager instance;
        return instance;
    }
    
    // 显示提示框
    void Show(const DialogConfig& config);
    
    // 快捷方法
    void ShowInfo(const std::string& message, const std::string& title = "提示");
    void ShowSuccess(const std::string& message, const std::string& title = "成功");
    void ShowWarning(const std::string& message, const std::string& title = "警告");
    void ShowError(const std::string& message, const std::string& title = "错误");
    void ShowConfirm(const std::string& message, 
                     std::function<void()> on_confirm,
                     std::function<void()> on_cancel = nullptr,
                     const std::string& title = "确认");
    
    // 关闭提示框
    void Close();
    
    // 状态查询
    bool IsVisible() const { return visible_; }
    const DialogConfig& GetConfig() const { return config_; }
    
    // 处理确认/取消
    void HandleConfirm();
    void HandleCancel();
    
private:
    DialogManager() = default;
    
    bool visible_ = false;
    DialogConfig config_;
};

// 创建提示框组件（用于渲染）
ftxui::Component CreateDialogComponent();

// 渲染提示框（用于叠加到其他页面上）
ftxui::Element RenderDialog();

// 包装器：为任意组件添加提示框支持
ftxui::Component WithDialog(ftxui::Component inner);