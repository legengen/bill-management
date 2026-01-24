add_library(screens)

set(screen_lists 
    LoginScreen
    RegisterScreen
    VisitScreen
    HomeScreen
    ChangePasswordScreen
    UserManageScreen
    EventManageScreen
    BillManageScreen
    BillCreateScreen
    BillQueryScreen
    ProfileScreen
    QueryByEventScreen
)
foreach(screen_name ${screen_lists})
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${screen_name}.cc)
        target_sources(screens
        PRIVATE
            ${screen_name}.cc
        PUBLIC
            FILE_SET HEADERS
            FILES
                ${screen_name}.h
        )
    endif()
endforeach()

target_link_libraries(screens
    PUBLIC
        cores
        services
        components
        ftxui::screen
        ftxui::dom
        ftxui::component
)