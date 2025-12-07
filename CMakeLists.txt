cmake_minimum_required(VERSION 3.23)
project(bill-management CXX)

# GoogleTest requires at least C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SQLite3 REQUIRED)

add_library(sqlite_orm INTERFACE)
target_include_directories(sqlite_orm INTERFACE 
    ${CMAKE_SOURCE_DIR}/external/sqlite_orm/include
)
target_link_libraries(sqlite_orm INTERFACE SQLite::SQLite3)

include(FetchContent)
FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
  GIT_TAG v6.1.9
)
FetchContent_MakeAvailable(ftxui)

option(BUILD_TESTS "Build the tests" FALSE)

add_subdirectory(src)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()