add_library(AuthService)

target_sources(AuthService
    PRIVATE 
        AuthService.cc

    PUBLIC 
        FILE_SET HEADERS
        FILES 
            AuthService.h
)

target_link_libraries(AuthService
    PRIVATE
        irepositories
        models
)