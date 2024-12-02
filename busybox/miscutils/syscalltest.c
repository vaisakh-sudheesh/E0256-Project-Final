/* vi: set sw=4 ts=4: */
/*
 * E0256 Project test code for busybox
 *
 *
 * Copyright (C) 2024 Vaisakh P S <vaisakhp@iisc.ac.in
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */

#include "libbb.h"

//config:config E0256_TESTS
//config:	bool "Enable test code for E0256 Project"
//config:	default y
//config:	help
//config:	Test code for E0256 Project

//applet:IF_E0256_TESTS(APPLET(syscalltest, BB_DIR_BIN, BB_SUID_DROP))

//kbuild:lib-$(CONFIG_E0256_TESTS) += syscalltest.o


//usage:#define syscalltest_trivial_usage NOUSAGE_STR
//usage:#define syscalltest_full_usage ""


#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>


extern const char sandbox_data[];
extern const long sandbox_init_data_size;

#define __NR_sandbox_dummycall      336
#define __NR_sandbox_init           337
#define __NR_sandbox_cleanup        338

int syscalltest_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int syscalltest_main(int argc UNUSED_PARAM, char **argv UNUSED_PARAM)
{
    int ret;
    ret = syscall(__NR_sandbox_init, sandbox_data, sandbox_init_data_size);

    // printf("syscalltest issuing syscall - size ofs\n");
    // printf ("size of int = %d\n", sizeof(int));
    // printf ("size of long = %d\n", sizeof(long));
    // printf ("size of long long = %d\n", sizeof(long long));
    // printf ("size of unsigned long = %d\n", sizeof(unsigned long));
    // printf ("size of unsigned long long = %d\n", sizeof(unsigned long long));

    for (int i = 1; i < 20; i++) {
        ret = syscall(__NR_sandbox_dummycall, i);
        if (ret < 0) {
            perror("syscall");
            return EXIT_FAILURE;
        }
        printf("syscall returned %d\n", ret);
    }
    printf("syscall returned %d\n", ret);
	return EXIT_SUCCESS;
}
