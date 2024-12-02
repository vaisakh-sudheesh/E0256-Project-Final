cmake_minimum_required(VERSION 3.20)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")


set(CMAKE_C_COMPILER /usr/bin/clang-19)
set(CMAKE_CXX_COMPILER /usr/bin/clang++-19)
set(CMAKE_LLVM_OPT /usr/bin/opt-19)
set(CMAKE_LLVM_LLC /usr/bin/llc-19)

project(E0256-Project LANGUAGES CXX C)

set (PRJ_LINUX_VERSION 6.11.9)
set (PRJ_BUSYBOX_VERSION 1.37.0)
set (PRJ_MBEDTLS_VERSION v3.6.2)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fdiagnostics-color=always")

# Set the build directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")

set(PRJ_WORKING_DIR ${CMAKE_BINARY_DIR}/working-dir)

set(PRJ_SOURCE_DIR      ${PRJ_WORKING_DIR}/source-dir)
set(PRJ_DOWNLOAD_DIR    ${PRJ_WORKING_DIR}/download-dir)
set(PRJ_BUILD_DIR       ${PRJ_WORKING_DIR}/build-dir)
set(PRJ_INSTALL_DIR     ${PRJ_WORKING_DIR}/install-dir)

set (PROJECT_TEST_OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/out-dir)
file(MAKE_DIRECTORY ${PROJECT_TEST_OUTPUT})


add_subdirectory(LibcListGen)
add_subdirectory(LLVM)
add_subdirectory(GraphLib)
add_subdirectory(test)
add_subdirectory(kernel/e0256-sandboxing/memgraphlib)

include(kernel)
include(busybox)
include(initrd)

FetchContent_MakeAvailable(linux-project busybox-project)


setup_kernel_workspace_fn(${PRJ_SOURCE_DIR}/linux-src 
                    ${CMAKE_CURRENT_SOURCE_DIR}/kernel
            )
setup_busybox_workspace_fn(${PRJ_SOURCE_DIR}/busybox-src 
                    ${CMAKE_CURRENT_SOURCE_DIR}/busybox 
            )


### Build Rules definitions
build_qemu_kernel(${PRJ_SOURCE_DIR}/linux-src 
            ${PRJ_BUILD_DIR}/linux-build 
            kernel_x86-qemu-minimal_defconfig 
            /usr/lib/llvm-19/bin/
            ${PROJECT_TEST_OUTPUT}/bzImage
            ${CMAKE_CURRENT_SOURCE_DIR}/src/kernel/
        )

## BusyBox
build_busybox(${PRJ_SOURCE_DIR}/busybox-src 
            ${PRJ_BUILD_DIR}/busybox-build
            ${PRJ_INSTALL_DIR}/bin/busybox
            ${CMAKE_CURRENT_SOURCE_DIR}/src/busybox/
        )


build_minimal_fs(${PRJ_SOURCE_DIR}/busybox-src 
            ${PRJ_BUILD_DIR}/busybox-build 
            ${PRJ_BUILD_DIR}/busybox-build/_install
            ${PROJECT_TEST_OUTPUT}/initramfs-busybox-x86.cpio.gz
            init.ref #init-noshell.ref            
            ${PROJECT_TEST_OUTPUT}/out/
        )

add_custom_target(setup_workspace ALL DEPENDS setup_kernel_workspace setup_busybox_workspace)

enable_testing()
add_test(NAME test_kernel
                COMMAND qemu-system-x86_64 
                        -kernel  ${PROJECT_TEST_OUTPUT}/bzImage
                        -initrd ${PROJECT_TEST_OUTPUT}/initramfs-busybox-x86.cpio.gz
                        -nographic -append "console=ttyS0"
                
)