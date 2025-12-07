add_subdirectory(services)
add_subdirectory(common)
add_subdirectory(data)
add_subdirectory(gui)

add_executable(bill-management main.cc)

target_link_libraries(bill-management
    PUBLIC
        cores
        gui
        services
        repositories_impl
        models
)

