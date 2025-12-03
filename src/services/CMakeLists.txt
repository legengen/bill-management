add_library(services)

target_sources(services
    PRIVATE 
        AuthService.cc
        UserService.cc
    PUBLIC 
        FILE_SET HEADERS
        FILES 
            AuthService.h
            UserService.h
)

target_link_libraries(services
    PRIVATE
        irepositories
        models
)