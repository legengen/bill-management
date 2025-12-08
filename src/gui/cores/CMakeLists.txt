add_library(cores)

set(core_lists 
    App
    Router
    Session
)
foreach(core_name ${core_lists})
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${core_name}.cc)
        target_sources(cores
        PRIVATE
            ${core_name}.cc
        PUBLIC
            FILE_SET HEADERS
            FILES
                ${core_name}.h
        )
    endif()
endforeach()

target_link_libraries(cores
    PUBLIC
        screens
        services
        models
        ftxui::screen
        ftxui::dom
        ftxui::component
)