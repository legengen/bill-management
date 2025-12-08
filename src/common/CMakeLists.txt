add_library(models INTERFACE)

target_sources(models 
    INTERFACE 
    FILE_SET HEADERS
    FILES
        models.h
)