add_library(components)

target_sources(components
    PRIVATE 
        UserInfoBar.cc
    PUBLIC 
        FILE_SET HEADERS
        FILES 
            UserInfoBar.h
)

target_link_libraries(components
    PUBLIC
        cores
        ftxui::screen
        ftxui::dom
        ftxui::component
)