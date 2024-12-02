#ifndef __DEFINITIONS_H_INCLUDED__
#define __DEFINITIONS_H_INCLUDED__

#define MEMORY_POOL_SIZE            (1024 * 1024)       // 1MB memory pool
#define GRAPH_META_MAGIC_NUMBER     (0xDEADBEEF)        // Magic number for graph metadata

#define NODE_TABLE_OFFSET           (sizeof(struct graph_metadata))

/**
 * Structure to represent the metadata of the graph.
 * @note This will be stored at the beginning of the memory pool.
 * 
 * @details The memory pool will be used to store the graph and will be created only once.
 *          The graph will be stored in the memory pool in a serialized format
 * 
 *          Layout of this memory pool:
 *              +-------------------------+
 *              |  Graph Metadata         |  <!-- This structure -->
 *              |                         |  <!-- Containing metadata and offsets to the other sections -->
 *              +-------------------------+
 *              |  Node Table             | <!-- This will be used to store the nodes and edges -->
*               |                         | 
 *              +-------------------------+
 *              |  Node Listings          | <!-- This will be used as a quick lookup table for nodes -->
 *              |                         | <!-- Will be populated after graph is finalized -->
 *              +-------------------------+
 * 
 * 
 */
struct graph_metadata {
    unsigned long   magic;                // Magic number to ensure sanity of data
    unsigned long   version;              // Version of the graph
    unsigned long   checksum;             // Checksum of the graph
    unsigned long   total_size;           // Total size of the memory pool

    unsigned char   graph_finalized;      // Flag to indicate if the graph is finalized

    unsigned char   reserved0[15];        // Reserved for future use
    unsigned char   reserved1[16];        // Reserved for future use
    


    /* Abstract Program State (Nodes) information */
    unsigned long   num_nodes;            // Number of nodes in the graph

    unsigned long   nodes_table_offset;   // Offset to the nodes table
    unsigned long   nodes_table_size;     // Size of the nodes table
};

/**
 * Structure to represent the memory pool.
 * @note This will be embedded in the program's memory space.
 * @note This will be used to store the graph.
 */
struct memory_pool {
    struct graph_metadata       metadata;
    /** Node Table to represent graph/automaton **/
};

/* Magic numbers to ensure sanity of data being processed */
#define NODE_MAGIC_NUMBER       (0xCAFEBABE)
#define EDGE_MAGIC_NUMBER       (0xBAADF00D)

/**
 * Structure to represent a library call (edges in the graph).
 */
struct libcalls {
    unsigned long magic;
    unsigned long libcallid;
    unsigned long next_progstate;
};

/**
 * Structure to represent a program state (Nodes in the graph).
 */
struct abstract_progstate{
    unsigned long magic;
    unsigned long id;
    unsigned long num_libcalls;
};

#define DEFAULT_NUMBER_OF_NODESLISTINGS  (4*1024)


#endif // __DEFINITIONS_H_INCLUDED__
