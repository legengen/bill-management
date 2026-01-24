#include "PageLayout.h"

using namespace ftxui;

Element CreatePageLayout(
    Element user_info,
    Element content_area,
    Element button_area,
    const LayoutConfig& config
) {
    auto content_with_padding = hbox({
        filler() | size(WIDTH, EQUAL, config.content_padding),
        content_area | flex,
        filler() | size(WIDTH, EQUAL, config.content_padding),
    }) | size(HEIGHT, EQUAL, config.content_height);

    auto button_with_padding = hbox({
        filler() | size(WIDTH, EQUAL, config.content_padding),
        button_area | flex,
        filler() | size(WIDTH, EQUAL, config.content_padding),
    });
    
    // 主内容
    auto main_content = vbox({
        text("欢迎来到智能家庭手账系统") | bold | center,
        text(""),
        user_info,
        text(""),
        content_with_padding | flex,
        text(""),
        button_with_padding,
        text(""),
    });
    
    // 外层布局
    return vbox({
        filler() | size(HEIGHT, EQUAL, 1),
        hbox({
            filler() | size(WIDTH, EQUAL, config.outer_padding),
            main_content | flex,
            filler() | size(WIDTH, EQUAL, config.outer_padding),
        }),
        filler() | size(HEIGHT, EQUAL, 1),
    }) | border;
}