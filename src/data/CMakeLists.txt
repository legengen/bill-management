add_library(irepositories INTERFACE)
target_sources(irepositories 
    INTERFACE 
        FILE_SET HEADERS
        FILES
            irepositories.h
)
target_link_libraries(irepositories INTERFACE models)

add_library(repositories_impl INTERFACE)

target_sources(repositories_impl
    PRIVATE
        AnnotationRepositoryImpl.cc
        BillRepositoryImpl.cc
        DatabaseORM.cc
        EventRepositoryImpl.cc
        UserRepositoryImpl.cc
    PUBLIC
        FILE_SET HEADERS
        FILES
            AnnotationRepositoryImpl.h
            BillRepositoryImpl.h
            DatabaseORM.h
            EventRepositoryImpl.h
            UserRepositoryImpl.h
            irepositories.h
)

target_link_libraries(repositories_impl
    INTERFACE 
        irepositories 
        sqlite_orm
        SQLite::SQLite3
)