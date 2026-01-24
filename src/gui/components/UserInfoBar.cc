#include "UserInfoBar.h"
#include "Session.h"
#include "App.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

using namespace ftxui;

std::string GetCurrentDate() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto tm_now = std::localtime(&time_t_now);
    
    std::ostringstream oss;
    oss << std::put_time(tm_now, "%Y-%m-%d");
    return oss. str();
}

std::string GetCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto tm_now = std::localtime(&time_t_now);
    
    std::ostringstream oss;
    oss << std::put_time(tm_now, "%H:%M:%S");
    return oss.str();
}

ftxui::Element RenderUserInfoBar(PageType page_type) {
    auto& session = Session::Instance();
    
    if (! session.IsLoggedIn()) {
        return hbox({
            text("未登录") | color(Color::Red),
        });
    }
    
    auto user = session.GetCurrentUser();
    
    // 基础信息（所有页面都显示）
    Elements info_elements;
    
    // 左侧：用户信息
    auto user_info = hbox({
        text("欢迎,"),
        text(user->phone + ","),
        text(user->role == "admin" ? "管理员" : "普通用户")
    });

    // 中间: 余额信息
    std::ostringstream balance_oss;
    balance_oss << std::fixed << std::setprecision(2) << user->balance;
    auto balance_info = hbox({
        filler(),
        text("当前余额:"),
        text(balance_oss.str()),
        filler(),
    });
    
    // 右侧：日期时间
    auto date_info = hbox({
        text("日期:") ,
        text(GetCurrentDate()),
        text(" "),
    });
    
    // 主信息栏
    auto main_bar = hbox({
        user_info,
        filler() | size(WIDTH, EQUAL, 1),
        balance_info,
        filler() | size(WIDTH, EQUAL, 1),
        date_info,
    });
    
    // 组合所有元素
    Elements all_elements;
    all_elements.push_back(main_bar);
    
    return vbox(all_elements);
}