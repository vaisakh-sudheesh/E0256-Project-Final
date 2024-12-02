FetchContent_Declare(
    busybox-project
    URL https://busybox.net/downloads/busybox-${PRJ_BUSYBOX_VERSION}.tar.bz2

    DOWNLOAD_DIR ${PRJ_DOWNLOAD_DIR}
    SOURCE_DIR ${PRJ_SOURCE_DIR}/busybox-src
    BINARY_DIR ${PRJ_BUILD_DIR}/busybox-build
    INSTALL_DIR ${PRJ_INSTALL_DIR}/
    STAMP_DIR ${PRJ_BUILD_DIR}/busybox-build
)

function (setup_busybox_workspace_fn busybox_src_dir project_root_dir)
    list (APPEND BUSYBOX_DEPS 
                ${busybox_src_dir}/miscutils/syscalltest-support.c
                ${busybox_src_dir}/miscutils/syscalltest.c
                ${busybox_src_dir}/shinit.ref
                ${busybox_src_dir}/init.ref
                ${busybox_src_dir}/init-noshell.ref
                ${busybox_src_dir}/package_initramfs.sh
                )
    add_custom_command(
        OUTPUT  
            ${BUSYBOX_DEPS}

        WORKING_DIRECTORY ${busybox_src_dir}

        COMMAND echo "Applying patches necessary for integration"
        COMMAND patch -p1 --ignore-whitespace < ${project_root_dir}/0001-E0256-Necessary-integrations.patch

        COMMAND echo "Mapping necessary implementation directories/files"
        COMMAND stat ${busybox_src_dir}/miscutils/syscalltest-support.c > /dev/null || ln -sf ${project_root_dir}/miscutils/syscalltest-support.c ${busybox_src_dir}/miscutils/syscalltest-support.c
        COMMAND stat ${busybox_src_dir}/miscutils/syscalltest.c > /dev/null || ln -sf ${project_root_dir}/miscutils/syscalltest.c ${busybox_src_dir}/miscutils/syscalltest.c
        COMMAND stat ${busybox_src_dir}/shinit.ref > /dev/null || ln -sf ${project_root_dir}/shinit.ref ${busybox_src_dir}/shinit.ref
        COMMAND stat ${busybox_src_dir}/init-noshell.ref > /dev/null || ln -sf ${project_root_dir}/init-noshell.ref ${busybox_src_dir}/init-noshell.ref
        COMMAND stat ${busybox_src_dir}/init.ref > /dev/null || ln -sf ${project_root_dir}/init.ref ${busybox_src_dir}/init.ref
        COMMAND stat ${busybox_src_dir}/package_initramfs.sh > /dev/null || ln -sf ${project_root_dir}/package_initramfs.sh ${busybox_src_dir}/package_initramfs.sh

        COMMENT "BusyBox Workspace Setup"
        USES_TERMINAL
    )
    add_custom_target(setup_busybox_workspace ALL DEPENDS ${BUSYBOX_DEPS})
endfunction(setup_busybox_workspace_fn)

function(build_busybox busybox_src_dir busybox_build_dir output project_root_dir)
    file(MAKE_DIRECTORY ${project_root_dir})

    add_custom_command(
        OUTPUT  ${output}
        WORKING_DIRECTORY ${busybox_src_dir}

        COMMAND echo "Configuring BusyBox"
        COMMAND CC=${CMAKE_C_COMPILER} make O=${busybox_build_dir} defconfig

        COMMAND echo "Building BusyBox with ${N} processors"
        COMMAND CC=${CMAKE_C_COMPILER} make O=${busybox_build_dir}

        COMMAND echo "Installing BusyBox"
        COMMAND CC=${CMAKE_C_COMPILER} make O=${busybox_build_dir} install > /dev/null

        COMMENT "BusyBox Build"
        USES_TERMINAL
    )
    add_custom_target(busybox_build ALL DEPENDS ${output})
    add_dependencies(busybox_build build_qemu_ubuntu_kernel)
endfunction(build_busybox)

