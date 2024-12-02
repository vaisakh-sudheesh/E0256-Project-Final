
FetchContent_Declare(
    linux-project
    URL https://www.kernel.org/pub/linux/kernel/v6.x/linux-${PRJ_LINUX_VERSION}.tar.xz

    DOWNLOAD_DIR ${PRJ_DOWNLOAD_DIR}
    SOURCE_DIR ${PRJ_SOURCE_DIR}/linux-src
    BINARY_DIR ${PRJ_BUILD_DIR}/linux-build
    INSTALL_DIR ${PRJ_INSTALL_DIR}/
    STAMP_DIR ${PRJ_BUILD_DIR}/linux-build
    CONFIGURE_COMMAND ./${CMAKE_CURRENT_SOURCE_DIR}/src/kernel/configure_kernel.sh
)

function(setup_kernel_workspace_fn kernel_src_dir project_root_dir)
    ## Create the directory for holding the kernel module code
    #file(MAKE_DIRECTORY ${kernel_src_dir}/security/e0256-sandboxing)

    list (APPEND KERNEL_DEPS 
                ${kernel_src_dir}/arch/x86/configs/kernel_x86-qemu-minimal_defconfig
                ${kernel_src_dir}/security/e0256-sandboxing)  

    add_custom_command(
        OUTPUT ${KERNEL_DEPS}
        WORKING_DIRECTORY ${kernel_src_dir}

        COMMAND echo "Applying patches necessary for integration"
        COMMAND patch -p1 --ignore-whitespace < ${project_root_dir}/0001-Necessary-changes-to-include-module.patch
        COMMAND patch -p0 --ignore-whitespace < ${project_root_dir}/syscall-tbl_${PRJ_LINUX_VERSION}.patch

        COMMAND echo "Mapping necessary implementation directories/files"        
        COMMAND stat ${kernel_src_dir}/arch/x86/configs/kernel_x86-qemu-minimal_defconfig  > /dev/null || ln -sf ${project_root_dir}/kernel_x86-qemu-minimal_defconfig  ${kernel_src_dir}/arch/x86/configs/kernel_x86-qemu-minimal_defconfig
        COMMAND stat ${kernel_src_dir}/security/e0256-sandboxing   > /dev/null || ln -sf ${project_root_dir}/e0256-sandboxing/  ${kernel_src_dir}/security/e0256-sandboxing
    )
    add_custom_target(setup_kernel_workspace ALL DEPENDS ${KERNEL_DEPS}  )
endfunction(setup_kernel_workspace_fn)


function(build_qemu_kernel kernel_src_dir kernel_build_dir defconfig toolchain_dir output project_root_dir)
    ###########################################################################
    ## Build rules for kernel
    ###########################################################################
    message(STATUS "Building QEMU Kernel")
    message(STATUS "Kernel Source Directory: ${kernel_src_dir}")
    message(STATUS "Kernel Build Directory: ${kernel_build_dir}")
    message(STATUS "Defconfig: ${defconfig}")
    message(STATUS "Output: ${output}")
    message(STATUS "Number of processors: ${CPU_COUNT}")

    ## Command to build the kernel
    add_custom_command(
        OUTPUT ${kernel_build_dir}/.config ${output}
        WORKING_DIRECTORY ${kernel_src_dir}

        COMMAND echo "Configuring Kernel"
        # COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/src/kernel/${defconfig} ${kernel_src_dir}/arch/x86/configs/
        COMMAND make O=${kernel_build_dir} LLVM=${toolchain_dir} ${defconfig}
        COMMAND scripts/config --file ${kernel_build_dir}/.config --disable SYSTEM_TRUSTED_KEYS
        COMMAND scripts/config --file ${kernel_build_dir}/.config --disable SYSTEM_REVOCATION_KEYS
        COMMAND scripts/config --file ${kernel_build_dir}/.config --enable CONFIG_DEBUG_INFO_BTF
        COMMAND scripts/config --file ${kernel_build_dir}/.config  --set-str CONFIG_SYSTEM_TRUSTED_KEYS \"\"
        COMMAND scripts/config --file ${kernel_build_dir}/.config  --set-str CONFIG_SYSTEM_REVOCATION_KEYS \"\"
        COMMAND make O=${kernel_build_dir}  LLVM=${toolchain_dir} -j${CPU_COUNT}
        COMMAND cp ${kernel_build_dir}/arch/x86_64/boot/bzImage ${output}
        COMMENT "Linux/QEMU Kernel Build"
        USES_TERMINAL
    )
    ## A custom target for building the kernel
    add_custom_target(build_qemu_ubuntu_kernel ALL DEPENDS ${output} )

endfunction(build_qemu_kernel)


