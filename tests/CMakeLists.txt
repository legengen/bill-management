
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

set(TEST_TARGETS
    auth_service_test
    user_service_test
    bill_service_test
    event_service_test
    bill_service_annotate_test
)

add_executable(auth_service_test auth_service_test.cc)
add_executable(user_service_test user_service_test.cc)
add_executable(bill_service_test bill_service_test.cc)
add_executable(event_service_test event_service_test.cc)
add_executable(bill_service_annotate_test bill_service_annotate_test.cc)

include(GoogleTest)

foreach(test_target ${TEST_TARGETS})
    target_link_libraries(${test_target}
        PRIVATE
            services
            models
            irepositories
            GTest::gtest_main
            GTest::gmock_main
    )
    
    target_include_directories(${test_target}
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src
    )
    
    gtest_discover_tests(${test_target})
endforeach()