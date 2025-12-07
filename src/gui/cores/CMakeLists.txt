add_library(cores)

target_sources(cores
    PRIVATE 
        App.cc
        Router.cc
        Session.cc
    PUBLIC 
        FILE_SET HEADERS
        FILES 
            App.h
            Router.h
            Session.h
)

target_link_libraries(cores
    PUBLIC
        screens
        services
        models
        ftxui::screen
        ftxui::dom
        ftxui::component
)