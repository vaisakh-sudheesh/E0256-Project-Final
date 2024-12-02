/* vi: set sw=4 ts=4: */
/*
 * Common syscalltest data and functions for busybox
 *
 * Copyright (C) 2024 Vaisakh P S <vaisakhp@iisc.ac.in>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */

#include "libbb.h"

const char sandbox_data[] __attribute__((section("sandbox_init_data_section"))) = "This is some data in sandbox_init_data, to test if it works";
const long sandbox_init_data_size = sizeof(sandbox_data);