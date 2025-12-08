add_library(repo_test_base INTERFACE)
target_sources(models 
    INTERFACE 
    FILE_SET HEADERS
    FILES
        DatabaseTestBase.h
)
target_link_libraries(repo_test_base
    INTERFACE
        repositories_impl
        GTest::gtest_main
)

set(REPO_TESTS
    user_repository_test
    bill_repository_test
    event_repository_test
    annotation_repository_test
)

foreach(test_name ${REPO_TESTS})
    
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${test_name}.cc)
        add_executable(${test_name} ${test_name}.cc)
        
        target_link_libraries(${test_name}
            PRIVATE
                repositories_impl
                repo_test_base
                GTest::gtest_main
                GTest::gmock_main
        )

        gtest_discover_tests(${test_name})
    endif()
endforeach()