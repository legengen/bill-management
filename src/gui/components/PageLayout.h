#pragma once
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <functional>

using namespace ftxui;

// 页面内容区域渲染函数类型
using ContentRenderer = std::function<Element()>;

// 统一页面布局参数
struct LayoutConfig {
    int outer_padding = 2;        // 外层左右留白
    int content_padding = 8;      // 内容区域左右留白
    int button_width = 12;        // 按钮固定宽度
    int content_height = 8;       // 内容区域高度
};

// 创建统一布局的页面
// title: 页面标题（可选，空则使用默认欢迎语）
// content_area: 中间内容区域
// button_area: 底部按钮区域
Element CreatePageLayout(
    Element user_info,
    Element content_area,
    Element button_area,
    const LayoutConfig& config = LayoutConfig{}
);