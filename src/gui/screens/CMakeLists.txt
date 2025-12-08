add_library(screens)

target_sources(screens
    PRIVATE
        LoginScreen.cc
        RegisterScreen.cc
        VisitScreen.cc
        HomeScreen.cc
    PUBLIC
        FILE_SET HEADERS
        FILES
            LoginScreen.h
            RegisterScreen.h
            VisitScreen.h
            HomeScreen.h
)

target_link_libraries(screens
    PUBLIC
        cores
        services
        components
        ftxui::screen
        ftxui::dom
        ftxui::component
)