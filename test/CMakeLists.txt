
set (PROJECT_TESTS ${CMAKE_CURRENT_LIST_DIR})
set (PROJECT_TEST_CODE ${PROJECT_TESTS}/testcodes)

file(MAKE_DIRECTORY ${PROJECT_TEST_OUTPUT}/ir)
file(MAKE_DIRECTORY ${PROJECT_TEST_OUTPUT}/out)
file(MAKE_DIRECTORY ${PROJECT_TEST_OUTPUT}/dot)
file(MAKE_DIRECTORY ${PROJECT_TEST_OUTPUT}/graph)

message(STATUS "Project test output directory: ${PROJECT_TEST_OUTPUT}")
message(STATUS "Project test code directory: ${PROJECT_TEST_CODE}")

## To iterate over all the test input codes and generate the corresponding IR and output
file(GLOB_RECURSE GRAPHGEN_FILES ${PROJECT_TEST_CODE}/*)
foreach(GRAPHGEN_FILE ${GRAPHGEN_FILES})
    get_filename_component(FILE_NAME ${GRAPHGEN_FILE} NAME)
    set(EXEC_OUT_FILENAME "${FILE_NAME}.out")
    set(FILE_NAME "${FILE_NAME}.ll")
    
    add_custom_command( 
        OUTPUT  ${PROJECT_TEST_OUTPUT}/ir/${FILE_NAME} 
                ${PROJECT_TEST_OUTPUT}/out/${EXEC_OUT_FILENAME}
                ${PROJECT_TEST_OUTPUT}/dot/${FILE_NAME}.dot
        COMMAND ${CMAKE_C_COMPILER} -S -emit-llvm ${GRAPHGEN_FILE} -o ${PROJECT_TEST_OUTPUT}/ir/${FILE_NAME} 
        COMMAND ${CMAKE_LLVM_OPT} -load-pass-plugin=${CMAKE_BINARY_DIR}/lib/libLibcCallGraphGen.so
                                  -passes="libc-sandboxing" 
                                  -cg-output-path ${PROJECT_TEST_OUTPUT}/dot
                                  -cg-output-name ${FILE_NAME} 
                                  ${PROJECT_TEST_OUTPUT}/ir/${FILE_NAME} 
                                  -o ${PROJECT_TEST_OUTPUT}/out/${EXEC_OUT_FILENAME}
        DEPENDS LibcCallGraphGen
        # WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Running custom command for ${GRAPHGEN_FILE}"
    )
    list (APPEND GRAPHGEN_IR_OUTPUTS ${PROJECT_TEST_OUTPUT}/ir/${FILE_NAME})
    list (APPEND GRAPHGEN_EXEC_OUTPUTS ${PROJECT_TEST_OUTPUT}/out/${EXEC_OUT_FILENAME})
    list (APPEND GRAPHGEN_DOT_OUTPUTS ${PROJECT_TEST_OUTPUT}/dot/${FILE_NAME}.dot)
endforeach()




## Custom target to generate graphs for all test files
add_custom_target(
    generate_graphs
    DEPENDS ${GRAPHGEN_DOT_OUTPUTS}
    COMMENT "Generating graphs for all test files"

    COMMAND dot -Tpng -O ${PROJECT_TEST_OUTPUT}/dot/*.dot 
    COMMAND stat ${PROJECT_TEST_OUTPUT}/dot/*.png > /dev/null 2>&1 && mv -f ${PROJECT_TEST_OUTPUT}/dot/*.png ${PROJECT_TEST_OUTPUT}/graph/ || echo "No graphs generated"

)

## Custom target to clean all the generated output files
add_custom_target(
    clean_output
    COMMENT "Removing all files under ${PROJECT_TEST_OUTPUT}"

    COMMAND ${CMAKE_COMMAND} -E rm -f ${PROJECT_TEST_OUTPUT}/ir/*
    COMMAND ${CMAKE_COMMAND} -E rm -f ${PROJECT_TEST_OUTPUT}/dot/*
    COMMAND ${CMAKE_COMMAND} -E rm -f ${PROJECT_TEST_OUTPUT}/graph/*
    COMMAND ${CMAKE_COMMAND} -E rm -f ${PROJECT_TEST_OUTPUT}/out/*
)

add_custom_target(graphgen_tests ALL DEPENDS generate_graphs)

add_custom_target(run_tests ALL DEPENDS clean_output LibcCallGraphGen graphgen_tests)
