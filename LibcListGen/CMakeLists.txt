project (LibcListGen)
add_executable(${PROJECT_NAME} LibcFuncDump.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE elf)
