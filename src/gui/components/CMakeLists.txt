add_library(components)

set(component_lists 
    UserInfoBar
)
foreach(component_name ${component_lists})
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${component_name}.cc)
        target_sources(components
        PRIVATE
            ${component_name}.cc
        PUBLIC
            FILE_SET HEADERS
            FILES
                ${component_name}.h
        )
    endif()
endforeach()

target_link_libraries(components
    PUBLIC
        cores
        ftxui::screen
        ftxui::dom
        ftxui::component
)