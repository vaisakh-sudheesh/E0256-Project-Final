#ifndef __UTILS_H_INCLUDED__
#define __UTILS_H_INCLUDED__


/* ============================================================================ */
/* ======================= START: Utility Functions/Code ====================== */
/* ============================================================================ */

#ifdef __KERNEL__
#define PRINT_ERROR_AND_EXIT(msg , ...) do { \
                    printk(KERN_ERR msg "\n"  __VA_OPT__(,) __VA_ARGS__); \
                    return; \
                } while(0)

#define PRINT_ERROR(msg , ...) do { \
                    printk(KERN_ERR msg "\n"  __VA_OPT__(,) __VA_ARGS__); \
                } while(0)

#define PRINT_DEBUG(msg , ...) do { \
                    printk(KERN_DEBUG msg "\n"  __VA_OPT__(,) __VA_ARGS__); \
                } while(0)

#define PRINT_INFO(msg, ...) do { \
                    printk(KERN_INFO msg "\n"  __VA_OPT__(,) __VA_ARGS__); \
                } while(0)

#define MALLOC(size)                kzalloc(size, GFP_KERNEL)
#define REALLOC(ptr, size)          krealloc(ptr, size, GFP_KERNEL)
#define FREE(ptr)                   kfree(ptr)
#define MEMSET(ptr, val, size)      memset(ptr, val, size)
#define MEMCPY(dst, src, size)      memcpy(dst, src, size)

#else

#define PRINT_ERROR_AND_EXIT(msg , ...) do { \
                    fprintf(stderr, msg "\n" __VA_OPT__(,) __VA_ARGS__); \
                    exit(EXIT_FAILURE); \
                } while(0)

#define PRINT_ERROR(msg , ...) do { \
                    fprintf(stderr, msg "\n"  __VA_OPT__(,) __VA_ARGS__); \
                } while(0)

#define PRINT_DEBUG(msg , ...) do { \
                    fprintf(stderr, msg "\n"  __VA_OPT__(,) __VA_ARGS__); \
                } while(0)

#define PRINT_INFO(msg, ...) do { \
                    fprintf(stderr, msg "\n"  __VA_OPT__(,) __VA_ARGS__); \
                } while(0)

#define MALLOC(size)                malloc(size)
#define REALLOC(ptr, size)          realloc(ptr, size)
#define FREE(ptr)                   free(ptr)
#define MEMSET(ptr, val, size)      memset(ptr, val, size)
#define MEMCPY(dst, src, size)      memcpy(dst, src, size)

#endif // __KERNEL__

/* ============================================================================ */
/* ======================== END: Utility Functions/Code ======================= */
/* ============================================================================ */

#endif // __UTILS_H_INCLUDED__
