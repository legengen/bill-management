
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Add test executable
add_executable(auth_service_test
  auth_service_test.cc
)

target_link_libraries(auth_service_test
    PRIVATE
      services
      irepositories
      models
      GTest::gtest_main
      GTest::gmock_main
)

add_executable(user_service_test
    user_service_test.cc
)

target_link_libraries(user_service_test
    PRIVATE
      services
      irepositories
      models
      GTest::gtest_main
      GTest::gmock_main
)

# BillService 测试
add_executable(bill_service_test
    bill_service_test.cc
)

target_link_libraries(bill_service_test
    PRIVATE
      services
      irepositories
      models
      GTest::gtest_main
      GTest::gmock_main
)

include(GoogleTest)
gtest_discover_tests(auth_service_test)