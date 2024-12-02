cmake_minimum_required(VERSION 3.28)
project(MemoryGraph)

##################### GoogleTest #####################
# GoogleTest requires at least C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


##################### MemGraph #####################

add_library(${PROJECT_NAME} STATIC memgraph.c)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/export)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/)

##################### TestCases #####################
enable_testing()

add_executable(test_memgraph tests/test_memgraph.cc)
target_link_libraries(test_memgraph  GTest::gtest_main ${PROJECT_NAME} )


include(GoogleTest)
gtest_discover_tests(test_memgraph WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})