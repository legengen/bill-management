add_library(irepositories INTERFACE)

target_include_directories(irepositories
    INTERFACE 
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(irepositories
    INTERFACE 
        models
)