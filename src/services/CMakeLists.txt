add_library(services)

target_sources(services
    PRIVATE 
        AuthService.cc
        UserService.cc
        BillService.cc
        EventService.cc
        StatisticsService.cc
    PUBLIC 
        FILE_SET HEADERS
        FILES 
            AuthService.h
            UserService.h
            BillService.h
            EventService.h
            StatisticsService.h
)

target_link_libraries(services
    PUBLIC
        repositories_impl
        models
)