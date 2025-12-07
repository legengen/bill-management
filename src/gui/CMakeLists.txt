add_library(gui INTERFACE)

target_link_libraries(gui
    INTERFACE
        cores 
        screens
        ftxui::screen
        ftxui::dom
        ftxui::component
)

add_subdirectory(cores)
add_subdirectory(screens)