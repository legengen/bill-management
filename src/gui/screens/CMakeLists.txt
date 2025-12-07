add_library(screens)

target_sources(screens
    PRIVATE
        LoginScreen.cc
        RegisterScreen.cc
        VisitScreen.cc
    PUBLIC
        FILE_SET HEADERS
        FILES
            LoginScreen.h
            RegisterScreen.h
            VisitScreen.h
)

target_link_libraries(screens
    PUBLIC
        cores
        services
        ftxui::screen
        ftxui::dom
        ftxui::component
)