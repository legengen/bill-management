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
    return oss.str();
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
    
    if (!session.IsLoggedIn()) {
        return hbox({
            filler(),
            text("未登录") | color(Color::Red),
            filler(),
        });
    }
    
    auto user = session.GetCurrentUser();

    auto user_info = hbox({
        text("欢迎,"),
        text(user->phone),
        text(","),
        text(user->role == "admin" ? "管理员" : "普通用户"),
    });

    std::ostringstream balance_oss;
    balance_oss << std::fixed << std::setprecision(2) << user->balance;
    auto balance_info = hbox({
        text("余额:"),
        text(balance_oss.str()),
    });

    auto date_info = hbox({
        text("日期:"),
        text(GetCurrentDate()),
    });

    return hbox({
        user_info,
        filler() | flex,
        balance_info,
        filler() | flex,
        date_info,
    });
}