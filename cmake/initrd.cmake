
function(build_minimal_fs busybox_src_dir busybox_build_dir busybox_install_dir output init_script addon_bins_dir)
    file(MAKE_DIRECTORY ${busybox_install_dir})
    add_custom_command(
        OUTPUT ${output}
        COMMAND ${busybox_src_dir}/package_initramfs.sh ${busybox_src_dir} ${busybox_build_dir} ${busybox_install_dir} ${output} ${init_script} ${addon_bins_dir}
        COMMENT "Creating Minimal Filesystem"
        USES_TERMINAL
    )
    add_custom_target(minimal_fs_build ALL DEPENDS ${output} busybox_build)
    add_dependencies(minimal_fs_build busybox_build LibcCallGraphGen run_tests)
endfunction(build_minimal_fs)