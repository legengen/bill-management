#include "UserManageScreen.h"
#include "App.h"
#include "Router.h"
#include "Session.h"
#include "UserInfoBar.h"
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

struct UserManageState {
    std::vector<model::User> users;
    int current_page = 0;
    int page_size = 3;
    int selected_row = 0;
    
    // 查询相关
    std::string search_phone;
    bool is_filtered = false;
    bool is_exact_search = false;
    
    // 余额修改弹窗
    bool show_edit_dialog = false;
    int editing_user_index = -1;
    std::string new_balance_str;
    
    void LoadAllUsers() {
        auto& user_service = App::Instance().GetUserService();
        users = user_service.GetAllUsers();
        is_filtered = false;
        is_exact_search = false;
        current_page = 0;
        selected_row = 0;
    }
    
    // 智能查询：11位精准，否则模糊
    void SearchByPhone() {
        if (search_phone.empty()) {
            DialogManager::Instance().ShowError("请输入手机号");
            return;
        }
        
        auto& user_service = App::Instance().GetUserService();
        
        if (search_phone.length() == 11) {
            // 精准查询
            auto user = user_service.GetUserByPhone(search_phone);
            users.clear();
            if (user.has_value()) {
                users.push_back(*user);
            }
            is_exact_search = true;
        } else {
            // 模糊查询
            users = user_service.QueryUserByPhone(search_phone);
            is_exact_search = false;
        }
        
        is_filtered = true;
        current_page = 0;
        selected_row = 0;
        
        if (users.empty()) {
            DialogManager::Instance().ShowInfo("未找到匹配的用户");
        }
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
            show_edit_dialog = true;
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
    
    void CloseEditDialog() {
        show_edit_dialog = false;
        editing_user_index = -1;
        new_balance_str.clear();
    }
    
    std::string FormatBalance(double balance) const {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << balance;
        return oss.str();
    }
};

ftxui::Component CreateUserManageScreen() {
    auto state = std::make_shared<UserManageState>();
    state->LoadAllUsers();
    
    auto search_input = Input(&state->search_phone, "输入手机号");
    
    auto search_btn = Button("查询", [state] {
        state->SearchByPhone();
    });
    
    auto show_all_btn = Button("显示全部", [state] {
        state->search_phone.clear();
        state->LoadAllUsers();
    });

    auto edit_btn = Button("修改余额", [state] {
        if (state->GetSelectedUser()) {
            state->OpenEditDialog();
        } else {
            DialogManager::Instance().ShowError("请先选择一个用户");
        }
    });
    
    auto return_btn = Button("返回", [] {
        Router::Instance().NavigateTo(Route::Home);
    });
    
    auto balance_input = Input(&state->new_balance_str, "新余额");
    
    auto dialog_confirm_btn = Button("确认", [state] {
        if (state->UpdateBalance()) {
            DialogManager::Instance().ShowSuccess("余额修改成功");
            state->CloseEditDialog();
        } else {
            DialogManager::Instance().ShowError("余额格式错误或修改失败");
        }
    });
    
    auto dialog_cancel_btn = Button("取消", [state] {
        state->CloseEditDialog();
    });
    
    auto main_container = Container::Vertical({
        Container::Horizontal({search_input, search_btn, show_all_btn}),
        Container::Horizontal({edit_btn, return_btn}),
    });
    
    auto dialog_container = Container::Vertical({
        balance_input,
        Container::Horizontal({dialog_confirm_btn, dialog_cancel_btn}),
    });
    
    auto tab_index = std::make_shared<int>(0);
    auto container = Container::Tab({
        main_container,
        dialog_container,
    }, tab_index.get());
    
    container = CatchEvent(container, [state](Event event) {
        if (state->show_edit_dialog) {
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
        if (event == Event::Return) {
            if (state->GetSelectedUser()) {
                state->OpenEditDialog();
                return true;
            }
        }
        return false;
    });

    return Renderer(container, [=] {
        *tab_index = state->show_edit_dialog ? 1 : 0;
        
        if (state->show_edit_dialog) {
            auto* user = (state->editing_user_index >= 0 && 
                         state->editing_user_index < static_cast<int>(state->users.size())) 
                         ? &state->users[state->editing_user_index] : nullptr;
            
            std::string user_info = user ? 
                ("用户: " + user->username + " (" + user->phone + ")") : "未知用户";
            std::string current_balance = user ? 
                ("当前余额: " + state->FormatBalance(user->balance)) : "";
            
            return vbox({
                filler(),
                hbox({
                    filler(),
                    vbox({
                        text("修改用户余额") | bold | center,
                        separator(),
                        text(""),
                        text(user_info) | center,
                        text(current_balance) | center | dim,
                        text(""),
                        hbox({
                            text("新余额: "),
                            balance_input->Render() | size(WIDTH, EQUAL, 15) | border,
                        }) | center,
                        text(""),
                        separator(),
                        hbox({
                            filler(),
                            dialog_confirm_btn->Render() | size(WIDTH, EQUAL, 10),
                            text("  "),
                            dialog_cancel_btn->Render() | size(WIDTH, EQUAL, 10),
                            filler(),
                        }),
                    }) | border | size(WIDTH, EQUAL, 40),
                    filler(),
                }),
                filler(),
            }) | border;
        }
        
        auto title = text("用户管理") | bold | center;
        auto user_info = RenderUserInfoBar(PageType::Home);
        
        Element filter_status;
        if (state->is_filtered) {
            filter_status = hbox({
                text(state->search_phone) | color(Color::Green),
                text("  找到 " + std::to_string(state->users.size()) + " 条记录"),
            }) | center;
        } else {
            filter_status = text("显示全部用户，共 " + std::to_string(state->users.size()) + " 条记录") | center | dim;
        }
        
        auto search_area = hbox({
            text("手机号: ") | center,
            search_input->Render() | size(WIDTH, EQUAL, 15) | border,
            text(" "),
            search_btn->Render() | size(WIDTH, EQUAL, 8),
            text(" "),
            show_all_btn->Render() | size(WIDTH, EQUAL, 10),
        }) | center;
        
        std::vector<std::vector<std::string>> table_data;
        table_data.push_back({"", "手机号", "用户名", "余额"});
        
        auto current_users = state->GetCurrentPageUsers();
        for (int i = 0; i < static_cast<int>(current_users.size()); i++) {
            const auto& user = current_users[i];
            std::string selector = (i == state->selected_row) ? ">" : " ";
            table_data.push_back({
                selector,
                user.phone,
                user.username,
                state->FormatBalance(user.balance)
            });
        }
        
        if (current_users.empty()) {
            table_data.push_back({"", "", "暂无用户数据", ""});
        }
        
        auto table = Table(table_data);
        table.SelectAll().Border(LIGHT);
        table.SelectRow(0).Decorate(bold);
        table.SelectColumn(0).DecorateCells(size(WIDTH, EQUAL, 3));
        table.SelectColumn(1).DecorateCells(size(WIDTH, EQUAL, 15));
        table.SelectColumn(2).DecorateCells(size(WIDTH, EQUAL, 12));
        table.SelectColumn(3).DecorateCells(size(WIDTH, EQUAL, 12));
        table.SelectAll().DecorateCells(center);
        
        if (state->selected_row >= 0 && state->selected_row < static_cast<int>(current_users.size())) {
            table.SelectRow(state->selected_row + 1).Decorate(bgcolor(Color::Blue));
        }
        
        auto table_element = table.Render();
        
        auto page_info = text("第 " + std::to_string(state->current_page + 1) + 
                              " / " + std::to_string(state->GetTotalPages()) + " 页  " ) | center;
        
        auto bottom_buttons = hbox({
            filler(),
            return_btn->Render() | size(WIDTH, EQUAL, 10),
            filler(),
        });
        
        auto content = vbox({
            title,
            text(""),
            filler(),
            user_info,
            text(""),
            search_area,
            filter_status,
            text(""),
            table_element | center,
            text(""),
            page_info,
            text(""),
            bottom_buttons,
            text("按←→切换页码")
        });
        
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
}