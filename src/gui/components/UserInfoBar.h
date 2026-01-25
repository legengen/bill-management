#pragma once
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <string>

// 页面类型枚举
enum class PageType {
    Visit,
    Login,
    Register,
    Home,
    ChangePassword,
    BillManage,
    BillQuery,
    BillStats,
    BillCreate,
    EventManage,
    UserManage,
    Profile,
    QueryByEvent,
};

// 渲染用户信息栏
ftxui::Element RenderUserInfoBar(PageType page_type = PageType::Home);

// 获取当前日期字符串
std::string GetCurrentDate();

// 获取当前时间字符串
std::string GetCurrentTime();