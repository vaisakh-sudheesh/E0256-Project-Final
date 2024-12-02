project (GraphLib)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

include(FetchContent)
FetchContent_Declare(
        graaflib
        GIT_REPOSITORY https://github.com/bobluppes/graaf.git
        GIT_TAG main
)
set(SKIP_TESTS ON)
set(SKIP_BENCHMARKS ON)
set(SKIP_EXAMPLES ON)
FetchContent_MakeAvailable(graaflib)


FetchContent_Declare(
    fmtlib
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 11.0.2
)

FetchContent_MakeAvailable(fmtlib)

##############################################

add_library(${PROJECT_NAME} STATIC GraphLib.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC Graaf::Graaf fmt::fmt)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/includes)


