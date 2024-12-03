/**
 * @file memgraph.c
 * @brief This file contains the implementation of the memory graph.
 * @author Vaisakh P S <vaisakhp@iisc.ac.in>
 */

#ifdef __KERNEL__

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/printk.h>
#include <linux/string.h>

#else

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#endif // __KERNEL__

#include "export/memgraph.h"
#include "utils.h"
#include "definitions.h"

/* Module level variables */
static struct memory_pool       *pool = NULL;
static char                     *pool_edge = NULL;

static struct abstract_progstate **nodes = NULL;
#ifndef __KERNEL__
static unsigned long num_nodes_listings = 0;
#endif // __KERNEL__

/* ========================== START: Pool Allocator  ========================== */
/*  This section is intended for both kernel and user utilization               */
/* ============================================================================ */

/**
 * This function will create a memory pool.
 * 
 * @note: The memory pool will be created only once.
 */
static void create_pool(void) {
    pool = (struct memory_pool *)MALLOC(MEMORY_POOL_SIZE);
    if (pool == NULL) {
        PRINT_ERROR_AND_EXIT("Failed to allocate memory for memory pool");
    }

    pool_edge = ((char*)pool) + NODE_TABLE_OFFSET;
    MEMSET(pool, 0, MEMORY_POOL_SIZE);

    pool->metadata.total_size            = MEMORY_POOL_SIZE;
    pool->metadata.version               = MEMPOOL_VERSION;
    pool->metadata.magic                 = GRAPH_META_MAGIC_NUMBER;    
    pool->metadata.nodes_table_offset    = NODE_TABLE_OFFSET;
    pool->metadata.num_nodes             = 0;
}

/**
 * This function will destroy the memory pool.
 */
static void destroy_pool(void) {
    if (pool) {
        FREE(pool);
        pool = NULL;
    }
}

/**
 * This function will return the graph in the memory pool.
 * @return char*: The pointer to the graph in the memory pool.
 */
void *get_graph(void) {
    return (void *)pool;
}

/**
 * To initialize the graph in the memory pool from a buffer
 */
void initialize_graph(void *data, unsigned long size) {
    if (pool == NULL) {
        create_pool();
    }

    // Copy the data to the memory pool
    MEMSET(pool, 0, size);
    if (size > MEMORY_POOL_SIZE) {
        PRINT_ERROR_AND_EXIT("Data size exceeds memory pool size");
    } else if ((data != NULL) && (size > 0)) {
        MEMCPY(pool, data, size);

        if (verify_graph() != 1) {
            PRINT_ERROR_AND_EXIT("Failed to initialize graph, verification failed.");
        } else {
            PRINT_INFO("Graph initialized from buffer and verified.\n");
        }
    } else {
        nodes = NULL;
    }

    PRINT_DEBUG("Graph initialized.\n");
}

void destroy_graph(void) {
    destroy_pool();
    if (nodes) {
        FREE(nodes);
        nodes = NULL;
    }
}


#ifndef __KERNEL__
/**
 * This function will dump the graph to a file.
 * @param filename: The name of the file to which the graph will be dumped.
 * 
 * @note: The file will be overwritten if it already exists.
 * @note: The graph will be dumped in binary format.
 * @note: The graph can be loaded back using the load_graph function.
 * @note: Essentially a serialization routine.
 * 
 */
void store_graph(const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        PRINT_ERROR_AND_EXIT("Failed to open file for writing");
    }

    size_t written = fwrite(pool, 1, MEMORY_POOL_SIZE, file);
    if (written != MEMORY_POOL_SIZE) {
        fclose(file);
        PRINT_ERROR_AND_EXIT("Failed to write memory pool to file");
    }

    fclose(file);
    PRINT_DEBUG("Graph stored to file %s\n", filename);
}

/**
 * This function will load the graph from a file.
 * @param filename: The name of the file from which the graph will be loaded.
 * 
 * @note: The file should be in the same format as dumped by the store_graph function.
 * @note: Essentially a deserialization routine.
 * 
 */
void load_graph(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        PRINT_ERROR_AND_EXIT("Failed to open file for reading");
    }

    // reinitialize the pool
    if (pool == NULL) {
        create_pool();
    }

    size_t read = fread(pool, 1, MEMORY_POOL_SIZE, file);
    if (read != MEMORY_POOL_SIZE) {
        fclose(file);
        PRINT_ERROR_AND_EXIT("Failed to read memory pool from file. Bytes read = %lu", read);
    }

    fclose(file);
    PRINT_DEBUG("Graph loaded from file %s, verifying it\n", filename);
    if(verify_graph() != 1) {
        PRINT_ERROR_AND_EXIT("Failed to load graph, verification failed.");
    }
}

#endif // __KERNEL__


/* ============================================================================ */
/* ========================== END: Pool Allocator  ============================ */
/* ============================================================================ */

/* ============================================================================ */
/* ========================== START: Graph Management ========================= */
/*  This section is intended for  userspace only - will be utilize by toolchain */
/* ============================================================================ */

#ifndef __KERNEL__
/**
 * This function will allocate memory for a new node in the memory pool.
 * @param num_predecessors: The number of predecessors of the node.
 * @return struct abstract_progstate*: The pointer to the newly allocated node.
 * 
 * @note: The memory will be allocated from the memory pool.
 * 
 */
void *alloc_node(unsigned long id, int num_successors, unsigned long *successor_node_list, unsigned long *libcall_list) {
    struct abstract_progstate *node = (struct abstract_progstate *)pool_edge;
    pool_edge += sizeof(struct abstract_progstate);

    // Check if we need to allocate more memory for the node listings
    if (nodes == NULL) {
        nodes = (struct abstract_progstate **)MALLOC(sizeof(struct abstract_progstate *) * DEFAULT_NUMBER_OF_NODESLISTINGS);
    } else if (pool->metadata.num_nodes >= num_nodes_listings) {
        num_nodes_listings += DEFAULT_NUMBER_OF_NODESLISTINGS;
        nodes = (struct abstract_progstate **)REALLOC(nodes, sizeof(struct abstract_progstate *) * num_nodes_listings);
    }

    // Initialize the node
    node->magic = NODE_MAGIC_NUMBER;
    node->id = id;
    node->num_libcalls = num_successors;

    // Add the node to the node listings (Lookup table), will be compiled during finalization
    nodes[id] = node;

    // Add the edges to the node
    if (num_successors > 0) {
        struct libcalls *libcalls = (struct libcalls *)pool_edge;
        pool_edge += num_successors * sizeof(struct libcalls);

        for (int i = 0; i < num_successors; i++) {
            libcalls[i].magic = EDGE_MAGIC_NUMBER;
            libcalls[i].libcallid = libcall_list[i];
            libcalls[i].next_progstate = successor_node_list[i];            
        }
    }
    // Update the number of nodes
    pool->metadata.num_nodes++;
    return node;
}

/**
 * This function will add a new node to the graph.
 * 
 */
void finalize_graph() {
    if (nodes == NULL) {
    } else {

    }
    // Mark the graph as finalized
    pool->metadata.graph_finalized = 1;
    
    // TODO: Calculate checksum on the finalized graph

    PRINT_DEBUG("Graph finalized\n");
}

#endif // __KERNEL__

/**
 * This function will verify the graph in the memory pool.
 * @return int: 1 if the graph is valid, else appropriate error code.
 * 
 */
int verify_graph(void) {
    if (pool == NULL) {
        PRINT_ERROR("Graph not initialized");
        return ERROR_INVALID_GRAPH;
    }

    if (pool->metadata.graph_finalized == 0) {
        PRINT_ERROR("Graph not finalized");
        return ERROR_INVALID_STATE;
    }

    if (pool->metadata.magic != GRAPH_META_MAGIC_NUMBER) {
        PRINT_ERROR("Invalid magic number in the graph");
        return ERROR_INVALID_GRAPH;
    }
    // TODO: Add more checks such checksum calculation/verification

    if (nodes == NULL) {
        nodes = (struct abstract_progstate **)MALLOC(sizeof(struct abstract_progstate *) * pool->metadata.num_nodes);
    }

    //Verify nodes and edges too
    char *start_node = ((char*)pool) + pool->metadata.nodes_table_offset;
    for (unsigned long i = 0; i < pool->metadata.num_nodes; i++) {
        struct abstract_progstate *node = (struct abstract_progstate *)start_node;
        // Update the node pointer
        nodes[i] = node;
        if (node->magic != NODE_MAGIC_NUMBER) {
            PRINT_ERROR("Invalid magic number in node %lu", i);
            return ERROR_INVALID_NODE;
        }
        start_node += sizeof(struct abstract_progstate);
        struct libcalls *libcalls = (struct libcalls *)(start_node);
        for (unsigned long j = 0; j < node->num_libcalls; j++) {
            if (libcalls[j].magic != EDGE_MAGIC_NUMBER) {
                PRINT_ERROR("Invalid magic number in edge %lu of node %lu", j, i);
                return ERROR_INVALID_EDGE;
            }
            start_node += sizeof(struct libcalls);
        }
    }
    return 1;
}

/* ============================================================================ */
/* ========================== END: Graph Management  ========================== */
/* ============================================================================ */

/* ============================================================================ */
/* ========================== START: Graph Querying  ============================ */
/* ============================================================================ */
static unsigned long current_progstate = 0;
static char *start_node = NULL;

void reset_progstate(void) {
    current_progstate = 0;
    start_node = (char *)nodes[current_progstate];
}

int is_state_transition_valid (unsigned long libccall) {
    struct abstract_progstate *node = (struct abstract_progstate *)start_node;
    struct libcalls *libcalls = (struct libcalls *)(start_node + sizeof(struct abstract_progstate));
    if (node->magic != NODE_MAGIC_NUMBER) {
        PRINT_ERROR("Invalid magic number in node %lu", current_progstate);
        return 0;
    }
    if (libcalls[0].magic != EDGE_MAGIC_NUMBER) {
        PRINT_ERROR("Invalid magic number in edge %lu of node %lu", libccall, current_progstate);
        return 0;
    }
    // PRINT_DEBUG("\t\tChecking transition: %lu(%lu) -> %lu", current_progstate, node->num_libcalls ,  libccall);
    for (unsigned long i = 0; i < node->num_libcalls; i++) {
        // PRINT_DEBUG("\t\t\tChecking edge: %lu -> %lu", libcalls[i].libcallid, libccall);
        if (libcalls[i].libcallid == libccall) {
            return libcalls[i].next_progstate;
        }
    }
    return 0;
}

int transition_to_state(unsigned long libccall) {
    if (current_progstate >= pool->metadata.num_nodes) {
        PRINT_ERROR("Invalid current state");
        return ERROR_INVALID_STATE;
    } else {
        unsigned long temp = is_state_transition_valid(libccall);
        // PRINT_DEBUG("\t\tTransition: %lu -> %lu -> %lu", current_progstate, libccall, temp);
        if (temp) {
            current_progstate = temp;
            start_node = (char *)nodes[current_progstate];
            return current_progstate;
        } else {
            PRINT_ERROR("Invalid state transition");
            return ERROR_INVALID_STATE;
        }
    }  
    return 0;
}


/* ============================================================================ */
/* ========================== END: Graph Querying  ============================ */
/* ============================================================================ */

