#ifndef __MEMGRAPH_H_INCLUDED__
#define __MEMGRAPH_H_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/* Constants */
#define MEMPOOL_VERSION     (0x00000001)    // Version of the memory pool library

/* Error codes */
#define ERROR_INVALID_GRAPH (-1)
#define ERROR_INVALID_NODE  (-2)
#define ERROR_INVALID_EDGE  (-3)
#define ERROR_INVALID_ARG   (-4)
#define ERROR_INVALID_STATE (-5)

/* Library APIs */

void initialize_graph(void *data, unsigned long size);
void destroy_graph(void);
void *get_graph(void);

#ifndef __KERNEL__
void store_graph(const char *filename);
void load_graph(const char *filename);
void *alloc_node(unsigned long id, int num_successors, unsigned long *successor_node_list, unsigned long *libcall_list);
void finalize_graph(void);
#endif // __KERNEL__
int verify_graph(void);

void reset_progstate(void);
int is_state_transition_valid (unsigned long libccall);
int transition_to_state(unsigned long libccall);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __MEMGRAPH_H_INCLUDED__

