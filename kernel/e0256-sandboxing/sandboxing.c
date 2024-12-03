#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include "memgraphlib/export/memgraph.h"

unsigned char *kernel_buffer = NULL;

SYSCALL_DEFINE2(sandbox_init, unsigned char*, data, unsigned long, size)
{
    unsigned long retval = 0;
#ifndef CONFIG_E0_256_SANDBOX_PROJECT
    printk(KERN_INFO "Sandbox Init Syscall:  NOT ENABLED\n");
    retval = -ENOSYS;
#else
    printk(KERN_INFO "Sandbox Init Syscal with buffer size %ld\n", size);
    if (size == 0 || !data) {
        printk(KERN_WARNING "Invalid buffer or size\n");
        return -EINVAL;
    }

    // Free up an existing allocation if it exists
    if (kernel_buffer) {
        kfree(kernel_buffer);
    }

    // Allocate memory in kernel space
    kernel_buffer = kmalloc(size, GFP_KERNEL);
    if (!kernel_buffer) {
        printk(KERN_ERR "Failed to allocate memory\n");
        return -ENOMEM;
    }

    // Copy data from user space to kernel space
    if (copy_from_user(kernel_buffer, data, size)) {
        printk(KERN_ERR "Failed to copy data from user space\n");
        kfree(kernel_buffer);
        return -EFAULT;
    }

    printk(KERN_INFO "Successfully copied data from user space\n");

    //printk(KERN_INFO "Data: %s\n", kernel_buffer);
    for (int i = 0; i < 20; i++) {
        printk(KERN_INFO "Data[%d]: %d\n", i, kernel_buffer[i]);
    }

    printk(KERN_INFO "Initializing in-memory graph\n");
    initialize_graph(kernel_buffer, size);
    reset_progstate();

#endif // CONFIG_E0_256_SANDBOX_PROJECT    
    return retval;
}


SYSCALL_DEFINE0(sandbox_cleanup)
{
    unsigned long retval = 0;
#ifndef CONFIG_E0_256_SANDBOX_PROJECT
    printk(KERN_INFO "Sandbox Init Syscall:  NOT ENABLED\n");
    retval = -ENOSYS;
#else
    printk(KERN_INFO "Sandbox Cleanup Syscall\n");

    // Free the allocated memory after use
    kfree(kernel_buffer);
#endif // CONFIG_E0_256_SANDBOX_PROJECT
    return retval;
}


SYSCALL_DEFINE1(sandbox_dummycall, unsigned long, number)
{
    unsigned long retval = 0;
#ifndef CONFIG_E0_256_SANDBOX_PROJECT
    printk(KERN_INFO "Sandbox Dummy Syscall: %ld - NOT ENABLED\n", number);
#else 
    printk(KERN_INFO "Sandbox Dummy Syscall: %ld\n", number);
    retval = number;

    if (is_state_transition_valid (number)) {
        printk(KERN_INFO "Sandbox Dummy Syscall: valid transition\n");
        retval = transition_to_state(number);
    } else {
        printk(KERN_INFO "Sandbox Dummy Syscall: invalid transition\n");
        do_exit(SIGKILL);
    }
#endif // CONFIG_E0_256_SANDBOX_PROJECT

    return retval;
}

