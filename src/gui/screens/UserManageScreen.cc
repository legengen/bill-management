#include "UserManageScreen.h"
#include "App.h"
#include "Router.h"
#include "Session.h"
#include "UserInfoBar.h"
#include "PageLayout.h"
#include "Dialog.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <memory>
#include <vector>
#include <iomanip>
#include <sstream>

using namespace ftxui;

// 弹窗类型
enum class UserDialogType {
    None,
    Search,
    EditBalance,
};

struct UserManageState {
    std::vector<model::User> users;
    int current_page = 0;
    int page_size = 3;
    int selected_row = 0;
    
    std::string search_phone;
    
    int editing_user_index = -1;
    std::string new_balance_str;
    
    UserDialogType dialog_type = UserDialogType::None;
    
    void LoadAllUsers() {
        auto& user_service = App::Instance().GetUserService();
        users = user_service.GetAllUsers();
        current_page = 0;
        selected_row = 0;
    }
    
    bool SearchByPhone() {
        if (search_phone.empty()) {
            LoadAllUsers();
            return true;
        }
        
        auto& user_service = App::Instance().GetUserService();
        
        if (search_phone.length() == 11) {
            auto user = user_service.GetUserByPhone(search_phone);
            users.clear();
            if (user.has_value()) {
                users.push_back(*user);
            }
        } else {
            users = user_service.QueryUserByPhone(search_phone);
        }
        
        current_page = 0;
        selected_row = 0;
        
        return !users.empty();
    }
    
    int GetTotalPages() const {
        if (users.empty()) return 1;
        return (users.size() + page_size - 1) / page_size;
    }
    
    std::vector<model::User> GetCurrentPageUsers() const {
        std::vector<model::User> result;
        int start = current_page * page_size;
        int end = std::min(start + page_size, static_cast<int>(users.size()));
        
        for (int i = start; i < end; i++) {
            result.push_back(users[i]);
        }
        return result;
    }
    
    void NextPage() {
        if (current_page < GetTotalPages() - 1) {
            current_page++;
            selected_row = 0;
        }
    }
    
    void PrevPage() {
        if (current_page > 0) {
            current_page--;
            selected_row = 0;
        }
    }
    
    void SelectNextRow() {
        auto page_users = GetCurrentPageUsers();
        if (selected_row < static_cast<int>(page_users.size()) - 1) {
            selected_row++;
        }
    }
    
    void SelectPrevRow() {
        if (selected_row > 0) {
            selected_row--;
        }
    }
    
    model::User* GetSelectedUser() {
        int global_index = current_page * page_size + selected_row;
        if (global_index >= 0 && global_index < static_cast<int>(users.size())) {
            return &users[global_index];
        }
        return nullptr;
    }
    
    void OpenEditDialog() {
        auto* user = GetSelectedUser();
        if (user) {
            editing_user_index = current_page * page_size + selected_row;
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(2) << user->balance;
            new_balance_str = oss.str();
            dialog_type = UserDialogType::EditBalance;
        }
    }
    
    bool UpdateBalance() {
        if (editing_user_index < 0 || editing_user_index >= static_cast<int>(users.size())) {
            return false;
        }
        
        try {
            double new_balance = std::stod(new_balance_str);
            if (new_balance < 0) {
                return false;
            }
            
            auto& user = users[editing_user_index];
            auto& user_service = App::Instance().GetUserService();
            
            user_service.SetBalance(user.id, new_balance);
            user.balance = new_balance;
            return true;
        } catch (...) {
            return false;
        }
    }
    
    void CloseDialog() {
        dialog_type = UserDialogType::None;
        editing_user_index = -1;
        new_balance_str.clear();
    }
    
    std::string FormatBalance(double balance) const {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << balance;
        return oss.str();
    }
    
    std::string GetRoleDisplay(const std::string& role) const {
        return role == "admin" ? "管理员" : "普通用户";
    }
};

ftxui::Component CreateUserManageScreen() {
    auto state = std::make_shared<UserManageState>();
    state->LoadAllUsers();
    
    auto tab_index = std::make_shared<int>(0);

    // ==================== 搜索弹窗 ====================
    auto search_input = Input(&state->search_phone, "输入手机号");

    auto search_confirm_btn = Button("搜索", [state] {
        if (state->SearchByPhone()) {
            state->dialog_type = UserDialogType::None;
            DialogManager::Instance().ShowSuccess(
                "找到 " + std::to_string(state->users.size()) + " 个用户"
            );
        } else {
            DialogManager::Instance().ShowError("未找到匹配的用户");
        }
    });

    auto search_cancel_btn = Button("取消", [state] {
        state->dialog_type = UserDialogType::None;
    });

    auto search_dialog_container = Container::Vertical({
        search_input,
        Container::Horizontal({search_confirm_btn, search_cancel_btn}),
    });

    // ==================== 修改余额弹窗 ====================
    auto balance_input = Input(&state->new_balance_str, "输入新余额");

    auto edit_confirm_btn = Button("确认", [state] {
        if (state->UpdateBalance()) {
            state->CloseDialog();
            DialogManager::Instance().ShowSuccess("余额修改成功");
        } else {
            DialogManager::Instance().ShowError("余额格式错误或修改失败");
        }
    });

    auto edit_cancel_btn = Button("取消", [state] {
        state->CloseDialog();
    });

    auto edit_dialog_container = Container::Vertical({
        balance_input,
        Container::Horizontal({edit_confirm_btn, edit_cancel_btn}),
    });

    // ==================== 主页面按钮 ====================
    auto search_btn = Button("搜索用户", [state] {
        state->search_phone.clear();
        state->dialog_type = UserDialogType::Search;
    });

    auto show_all_btn = Button("显示全部", [state] {
        state->search_phone.clear();
        state->LoadAllUsers();
    });

    auto edit_btn = Button("修改余额", [state] {
        if (state->GetSelectedUser()) {
            state->OpenEditDialog();
        } else {
            DialogManager::Instance().ShowError("请先选择用户");
        }
    });

    auto return_btn = Button("返回", [] {
        Router::Instance().NavigateTo(Route::Home);
    });

    // ==================== 主容器 ====================
    auto main_container = Container::Horizontal({
        search_btn,
        show_all_btn,
        edit_btn,
        return_btn,
    });

    // ==================== Tab 容器 ====================
    auto container = Container::Tab({
        main_container,          // 0: 主页面
        search_dialog_container, // 1: 搜索弹窗
        edit_dialog_container,   // 2: 修改余额弹窗
    }, tab_index.get());

    LayoutConfig config;
    config.button_width = 12;
    config.content_padding = 4;
    config.outer_padding = 2;
    config.content_height = 14;

    const int COL_ID = 6;
    const int COL_PHONE = 14;
    const int COL_USERNAME = 12;
    const int COL_ROLE = 10;
    const int COL_BALANCE = 12;

    return Renderer(container, [=]() {
        // 更新 tab_index
        switch (state->dialog_type) {
            case UserDialogType::None:        *tab_index = 0; break;
            case UserDialogType::Search:      *tab_index = 1; break;
            case UserDialogType::EditBalance: *tab_index = 2; break;
        }
        
        auto user_info = RenderUserInfoBar();

        std::vector<std::vector<std::string>> table_data;
        table_data.push_back({"ID", "手机号", "用户名", "角色", "余额"});
        
        auto page_users = state->GetCurrentPageUsers();
        for (const auto& user : page_users) {
            table_data.push_back({
                std::to_string(user.id),
                user.phone,
                user.username,
                state->GetRoleDisplay(user.role),
                state->FormatBalance(user.balance),
            });
        }

        while (table_data.size() < static_cast<size_t>(state->page_size + 1)) {
            table_data.push_back({"", "", "", "", ""});
        }
        
        auto table = Table(table_data);

        table.SelectAll().SeparatorVertical(LIGHT);
        table.SelectAll().SeparatorHorizontal(LIGHT);
        table.SelectAll().Border(LIGHT);

        table.SelectColumn(0).DecorateCells(size(WIDTH, EQUAL, COL_ID));
        table.SelectColumn(1).DecorateCells(size(WIDTH, EQUAL, COL_PHONE));
        table.SelectColumn(2).DecorateCells(size(WIDTH, EQUAL, COL_USERNAME));
        table.SelectColumn(3).DecorateCells(size(WIDTH, EQUAL, COL_ROLE));
        table.SelectColumn(4).DecorateCells(size(WIDTH, EQUAL, COL_BALANCE));

        table.SelectAll().DecorateCells(center);
        table.SelectRow(0).Decorate(bold);
        table.SelectRow(0).DecorateCells(bgcolor(Color::Blue));
        
        if (state->selected_row >= 0 && state->selected_row < static_cast<int>(page_users.size())) {
            table.SelectRow(state->selected_row + 1).DecorateCells(bgcolor(Color::GrayDark));
        }
        
        auto table_element = table.Render();
        
        auto page_info = hbox({
            text("第 " + std::to_string(state->current_page + 1) + 
                 "/" + std::to_string(state->GetTotalPages()) + " 页"),
            text("  "),
            text("← → 翻页  ↑ ↓ 选行") | dim,
        }) | center;
        
        auto content_area = vbox({
            text(""),
            table_element | center,
            text(""),
            page_info,
        });
        
        auto button_area = hbox({
            search_btn->Render() | size(WIDTH, EQUAL, config.button_width),
            text(" "),
            show_all_btn->Render() | size(WIDTH, EQUAL, config.button_width),
            text(" "),
            edit_btn->Render() | size(WIDTH, EQUAL, config.button_width),
            filler(),
            return_btn->Render() | size(WIDTH, EQUAL, config.button_width),
        });
        
        auto main_page = CreatePageLayout(user_info, content_area, button_area, config);
        
        // ==================== 弹窗渲染 ====================
        if (state->dialog_type == UserDialogType::Search) {
            auto dialog_box = vbox({
                text("搜索用户") | bold | center,
                separator(),
                text(""),
                hbox({
                    text("手机号: ") | vcenter,
                    search_input->Render() | size(WIDTH, EQUAL, 15) | border,
                }) | center,
                text(""),
                separator(),
                hbox({
                    filler(),
                    search_confirm_btn->Render() | size(WIDTH, EQUAL, 10),
                    text("  "),
                    search_cancel_btn->Render() | size(WIDTH, EQUAL, 10),
                    filler(),
                }),
            }) | border | clear_under | center;
            
            return dbox({
                main_page | dim,
                dialog_box,
            });
        }
        
        if (state->dialog_type == UserDialogType::EditBalance) {
            auto* user = state->GetSelectedUser();
            std::string user_phone = user ? user->phone : "";
            std::string user_name = user ? user->username : "";
            std::string current_balance = user ? state->FormatBalance(user->balance) : "0.00";
            
            auto dialog_box = vbox({
                hbox({
                    text("用户: ") | size(WIDTH, EQUAL, 5),
                    text(user_phone) | border,
                }) | center,
                text(""),
                hbox({
                    text("新余额: ") | size(WIDTH, EQUAL, 5),
                    balance_input->Render() | size(WIDTH, EQUAL, 15) | border,
                }) | center,
                hbox({
                    filler(),
                    edit_confirm_btn->Render() | size(WIDTH, EQUAL, 10),
                    text("  "),
                    edit_cancel_btn->Render() | size(WIDTH, EQUAL, 10),
                    filler(),
                }),
            }) | border | clear_under | center;
            
            return dbox({
                main_page | dim,
                dialog_box,
            });
        }
        
        return main_page;
    }) | CatchEvent([state](Event event) {
        if (state->dialog_type != UserDialogType::None) {
            if (event == Event::Escape) {
                state->CloseDialog();
                return true;
            }
            return false;
        }
        
        if (event == Event::ArrowUp) {
            state->SelectPrevRow();
            return true;
        }
        if (event == Event::ArrowDown) {
            state->SelectNextRow();
            return true;
        }
        if (event == Event::ArrowLeft) {
            state->PrevPage();
            return true;
        }
        if (event == Event::ArrowRight) {
            state->NextPage();
            return true;
        }
        
        return false;
    });
}