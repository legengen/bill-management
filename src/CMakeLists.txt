add_subdirectory(services)
add_subdirectory(common)
add_subdirectory(data)

add_executable(bill-management)

target_sources(bill-management
    PRIVATE
        main.cc
)

target_link_libraries(bill-management
    PRIVATE
        AuthService
)

