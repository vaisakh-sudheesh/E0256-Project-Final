#include <filesystem>
#include <iostream>
#include <fstream>
#include <tuple>
#include <gtest/gtest.h>
#include <memgraph.h>

#include "../definitions.h"
#include "test_utils.h"

/* ------------------------------------------------------------------------- */
/* -------------------------- BASIC TEST CASES ----------------------------- */
/* ------------------------------------------------------------------------- */
TEST(MemGraph_BasicUnit, InitializeGraph) {
    initialize_graph(NULL, 0);
    void *graph = get_graph();
    ASSERT_NE(graph, nullptr) << "Graph not initialized";
    destroy_graph ();
    graph = get_graph();
    ASSERT_EQ(graph, nullptr) << "Graph not destroyed";
}

TEST(MemGraph_BasicUnit, GetGraph) {
    void *graph = get_graph();
    ASSERT_EQ(graph, nullptr) << "Graph seems to be initialized";
    destroy_graph ();
    graph = get_graph();
    ASSERT_EQ(graph, nullptr) << "Graph not destroyed";
}

TEST(MemGraph_BasicUnit, DestroyGraph1) {
    destroy_graph ();
    void *graph = get_graph();
    ASSERT_EQ(graph, nullptr) << "Graph not destroyed";
}

TEST(MemGraph_BasicUnit, DestroyGraph2) {
    initialize_graph(NULL, 0);
    void *graph = get_graph();
    ASSERT_NE(graph, nullptr) << "Graph not initialized";
    destroy_graph ();
    graph = get_graph();
    ASSERT_EQ(graph, nullptr) << "Graph not destroyed";
    destroy_graph ();
    ASSERT_EQ(graph, nullptr) << "Graph not destroyed";
}

TEST (MemGraph_BasicUnit, PoolInitCheck1) {
    initialize_graph(NULL, 0);
    void *graph = get_graph();
    ASSERT_NE(graph, nullptr) << "Graph not initialized";
    struct memory_pool *pool = (struct memory_pool *)graph;
    EXPECT_EQ(pool->metadata.magic, GRAPH_META_MAGIC_NUMBER) << "Invalid magic number in the graph";
    EXPECT_EQ(pool->metadata.version, MEMPOOL_VERSION) << "Invalid version of the graph";
    EXPECT_EQ(pool->metadata.graph_finalized, 0) << "Graph not finalized";
    finalize_graph();
    EXPECT_EQ(pool->metadata.graph_finalized, 1) << "Graph not finalized";
    graph = get_graph();
    destroy_graph ();
    EXPECT_NE(graph, nullptr) << "Graph not destroyed";
}

TEST (MemGraph_BasicUnit, PoolInitCheck2) {
    initialize_graph(NULL, 0);
    void *graph = get_graph();
    ASSERT_NE(graph, nullptr) << "Graph not initialized";
    struct memory_pool *pool = (struct memory_pool *)graph;
    EXPECT_EQ(pool->metadata.magic, GRAPH_META_MAGIC_NUMBER) << "Invalid magic number in the graph";
    EXPECT_EQ(pool->metadata.version, MEMPOOL_VERSION) << "Invalid version of the graph";
    EXPECT_EQ(pool->metadata.graph_finalized, 0) << "Graph not finalized";
    finalize_graph();
    EXPECT_EQ(pool->metadata.graph_finalized, 1) << "Graph not finalized";
    graph = get_graph();
    // Copy the memory pool to a buffer
    void *buffer = malloc(pool->metadata.total_size);
    memcpy(buffer, graph, pool->metadata.total_size);

    // Now the graph may be destroyed
    destroy_graph ();
    
    EXPECT_NE(graph, nullptr) << "Graph not destroyed";

    // Reinitialize the graph from the buffer
    initialize_graph(buffer, ((struct memory_pool*)buffer)->metadata.total_size);
    // Freeing up the temporary buffer
    free(buffer);
    graph = get_graph();
    ASSERT_NE(graph, nullptr) << "Graph not Re-initialized";
    pool = (struct memory_pool *)graph;
    EXPECT_EQ(pool->metadata.magic, GRAPH_META_MAGIC_NUMBER) << "Invalid magic number in the reinitialized graph";
    EXPECT_EQ(pool->metadata.version, MEMPOOL_VERSION) << "Invalid version of the reinitialized graph";
    EXPECT_EQ(pool->metadata.graph_finalized, 1) << "Finalized marker not set in reinitialized graph";
    destroy_graph ();
    graph = get_graph();
    EXPECT_EQ(graph, nullptr) << "Graph not destroyed";
}

TEST (MemGraph_BasicUnit, VerifyCheckBasicSanity) {
  EXPECT_EQ(verify_graph(), ERROR_INVALID_GRAPH) << "Expected failure return code not received";
  initialize_graph(NULL, 0);
  EXPECT_EQ(verify_graph(), ERROR_INVALID_STATE) << "Expected success return code not received";
  finalize_graph();
  EXPECT_EQ(verify_graph(), 1) << "Expected success return code not received";
}


/* ------------------------------------------------------------------------- */
/* --------------------- GRAPH CREATION TEST CASES ------------------------- */
/* ------------------------------------------------------------------------- */

TEST(MemGraph_GraphCreation, CreateGraph) {
    initialize_graph(NULL, 0);
    void *graph = get_graph();
    ASSERT_NE(graph, nullptr) << "Graph not initialized";

    unsigned long node_list[2] = {0, 1};
    unsigned long libcall_list[2] = {0, 1};
    struct abstract_progstate *node = (struct abstract_progstate *)alloc_node(0, sizeof(node_list)/sizeof(unsigned long), node_list, libcall_list);
    ASSERT_NE(node, nullptr) << "Node not allocated";
    EXPECT_EQ(node->magic, NODE_MAGIC_NUMBER) << "Invalid magic number in the node";
    EXPECT_EQ(node->id, 0) << "Invalid node id";
    EXPECT_EQ(node->num_libcalls, 2) << "Invalid number of libcalls in the node";
    // Check the successors edges of the created node
    struct libcalls *libcalls = (struct libcalls *)((char *)node + sizeof(struct abstract_progstate));
    for (int i = 0; i < 2; i++) {
        EXPECT_EQ(libcalls[i].magic, EDGE_MAGIC_NUMBER) << "Invalid magic number in the edge";
        EXPECT_EQ(libcalls[i].libcallid, i) << "Invalid libcall id in the edge";
        EXPECT_EQ(libcalls[i].next_progstate, i) << "Invalid next progstate in the edge";
    }

    // Check if the node is added to the graph
    graph = get_graph();
    ASSERT_NE(graph, nullptr) << "Graph not initialized";
    struct memory_pool *pool = (struct memory_pool *)graph;
    EXPECT_EQ(pool->metadata.num_nodes, 1) << "Invalid number of nodes in the graph";


    unsigned long node_list1[2] = {2, 1};
    unsigned long libcall_list1[2] = {100, 12};
    struct abstract_progstate *node1 = (struct abstract_progstate *)alloc_node(1, sizeof(node_list1)/sizeof(unsigned long), node_list1, libcall_list1);
    ASSERT_NE(node1, nullptr) << "Node not allocated";
    EXPECT_EQ(node1->magic, NODE_MAGIC_NUMBER) << "Invalid magic number in the node";
    EXPECT_EQ(node1->id, 1) << "Invalid node id";
    EXPECT_EQ(node1->num_libcalls, 2) << "Invalid number of libcalls in the node";
    // Check the successors edges of the created node
    struct libcalls *libcalls1 = (struct libcalls *)((char *)node1 + sizeof(struct abstract_progstate));
    for (int i = 0; i < 2; i++) {
        EXPECT_EQ(libcalls1[i].magic, EDGE_MAGIC_NUMBER) << "Invalid magic number in the edge";
        EXPECT_EQ(libcalls1[i].libcallid, libcall_list1[i]) << "Invalid libcall id in the edge";
        EXPECT_EQ(libcalls1[i].next_progstate, node_list1[i]) << "Invalid next progstate in the edge";
    }

    // Check if the node is added to the graph
    graph = get_graph();
    ASSERT_NE(graph, nullptr) << "Graph not initialized";
    pool = (struct memory_pool *)graph;
    EXPECT_EQ(pool->metadata.num_nodes, 2) << "Invalid number of nodes in the graph";

    unsigned long node_list2[3] = {0, 1, 2};
    unsigned long libcall_list2[3] = {10, 1, 10};
    struct abstract_progstate *node2 = (struct abstract_progstate *)alloc_node(20, sizeof(node_list2)/sizeof(unsigned long), node_list2, libcall_list2);
    ASSERT_NE(node2, nullptr) << "Node not allocated";
    EXPECT_EQ(node2->magic, NODE_MAGIC_NUMBER) << "Invalid magic number in the node";
    EXPECT_EQ(node2->id, 20) << "Invalid node id";
    EXPECT_EQ(node2->num_libcalls, 3) << "Invalid number of libcalls in the node";
    // Check the successors edges of the created node
    struct libcalls *libcalls2 = (struct libcalls *)((char *)node2 + sizeof(struct abstract_progstate));
    for (int i = 0; i < 3; i++) {
        EXPECT_EQ(libcalls2[i].magic, EDGE_MAGIC_NUMBER) << "Invalid magic number in the edge";
        EXPECT_EQ(libcalls2[i].libcallid, libcall_list2[i]) << "Invalid libcall id in the edge";
        EXPECT_EQ(libcalls2[i].next_progstate, node_list2[i]) << "Invalid next progstate in the edge";
    }

    destroy_graph ();
    graph = get_graph();
    ASSERT_EQ(graph, nullptr) << "Graph not destroyed";
}

TEST(MemGraph_GraphCreation, GraphVerify) {
    initialize_graph(NULL, 0);
    void *graph = get_graph();
    ASSERT_NE(graph, nullptr) << "Graph not initialized";

    unsigned long node_list[10] = {0};
    unsigned long libcall_list[10] = {0};

    node_list[0] = 0; libcall_list[0] = 10;
    node_list[1] = 1; libcall_list[1] = 1;
    node_list[2] = 2; libcall_list[2] = 10;
    alloc_node(0, 3, node_list, libcall_list);

    node_list[0] = 1; libcall_list[0] = 20;
    node_list[1] = 2; libcall_list[1] = 10;
    alloc_node(1, 2, node_list, libcall_list);

    finalize_graph();
    graph = get_graph();
    ASSERT_NE(graph, nullptr) << "Graph not finalized";
    EXPECT_EQ(verify_graph(), 1) << "Graph verification failed";    
}


/* ------------------------------------------------------------------------- */
/* ------------------- GRAPH SAVE & LOAD TEST CASES ------------------------ */
/* ------------------------------------------------------------------------- */

TEST(MemGraph_GraphSaveLoad, Verify1) {
  initialize_graph(NULL, 0);
  void *graph = get_graph();

  unsigned long node_list[10] = {0};
  unsigned long libcall_list[10] = {0};

  node_list[0] = 0; libcall_list[0] = 10;
  node_list[1] = 1; libcall_list[1] = 1;
  node_list[2] = 2; libcall_list[2] = 10;
  alloc_node(0, 3, node_list, libcall_list);

  node_list[0] = 1; libcall_list[0] = 20;
  node_list[1] = 2; libcall_list[1] = 10;
  alloc_node(1, 2, node_list, libcall_list);

  finalize_graph();
  store_graph("test.graph");
  destroy_graph ();
  // Check if the file "test.graph" is created
  std::ifstream infile("test.graph");
  EXPECT_EQ(infile.good(), true) << "Graph file not created";
  infile.close();

  load_graph("test.graph");
  graph = get_graph();
  ASSERT_NE(graph, nullptr) << "Graph not loaded";

  // Remove the file "test.graph"
  int result = remove("test.graph");
  EXPECT_EQ(result, 0) << "Graph file not removed";

}


/* ------------------------------------------------------------------------- */
/* -------------------- COMPREHENSIVE GRAPH TESTS -------------------------- */
/* ------------------------------------------------------------------------- */

TEST(MemGraph_Comprehensive, GraphRead1) {
    // Read the test data from input file & initialize the grap
    ASSERT_TRUE(read_test_data ("tests/test_data/test-case-1.txt"));
    initialize_graph(NULL, 0);
    void *graph = get_graph();
    EXPECT_NE (graph, nullptr) << "Graph not initialized";

    // Populate the edges and nodes in the graph
    for (unsigned long i = 0; i < tc_data_size; i++) {
        unsigned long *edge_list = &tc_edge_listings[i][0];
        unsigned long *libcall_list = &tc_libcall_listings[i][0];
        alloc_node(i, tc_edge_listings[i].size(), edge_list, libcall_list);
        graph = get_graph();
        ASSERT_NE (graph, nullptr) << "Graph not initialized";
        struct memory_pool *pool = (struct memory_pool *)graph;
        EXPECT_EQ(pool->metadata.num_nodes, i+1) << "Invalid number of nodes in the graph";
    }

    // Verify the graph
    finalize_graph();
    graph = get_graph();
    struct memory_pool *pool = (struct memory_pool *)graph;
    ASSERT_NE (graph, nullptr) << "Graph not finalized";
    EXPECT_EQ(verify_graph(), 1) << "Graph verification failed";
    EXPECT_EQ(pool->metadata.num_nodes, tc_data_size) << "Invalid number of nodes in the graph";

    // Cleanup the graph
    destroy_graph ();
    cleanup_test_data();
    sleep(1);
}

class MemGraph_ComprehensiveParam :
    public testing::TestWithParam<std::tuple<bool,std::string>> {
};


TEST_P(MemGraph_ComprehensiveParam, GraphReadnTraverseln) {
    // Read the test data from input file & initialize the graph
    bool fileTest;
    std::string filename ;
    
    std::tie(fileTest, filename) = GetParam();
    ASSERT_TRUE(read_test_data (filename.c_str()));
    initialize_graph(NULL, 0);
    void *graph = get_graph();
    EXPECT_NE (graph, nullptr) << "Graph not initialized";

    // Populate the edges and nodes in the graph
    for (unsigned long i = 0; i < tc_data_size; i++) {
        unsigned long *edge_list = &tc_edge_listings[i][0];
        unsigned long *libcall_list = &tc_libcall_listings[i][0];
        alloc_node(i, tc_edge_listings[i].size(), edge_list, libcall_list);
        graph = get_graph();
        ASSERT_NE (graph, nullptr) << "Graph not initialized";
        struct memory_pool *pool = (struct memory_pool *)graph;
        EXPECT_EQ(pool->metadata.num_nodes, i+1) << "Invalid number of nodes in the graph";
    }

    // Verify the graph
    finalize_graph();
    graph = get_graph();
    struct memory_pool *pool = (struct memory_pool *)graph;
    ASSERT_NE (graph, nullptr) << "Graph not finalized";
    EXPECT_EQ(verify_graph(), 1) << "Graph verification failed";
    EXPECT_EQ(pool->metadata.num_nodes, tc_data_size) << "Invalid number of nodes in the graph";

    // Traverse the graph and see if the transitions are valid
    
    unsigned long current_state = 0;
    unsigned long  test_result;
    for (unsigned long i = 0; i < test_cases.size(); i++) {
        test_result = 1;
        // std::cout << "Test case: " << i << "Size" <<   test_cases[i].size()<< std::endl;  
        reset_progstate();
        for (unsigned long j = 0; j < test_cases[i].size(); j++) {
            unsigned long libcall = test_cases[i][j];
            // std::cout << "("<<j<<") Transition: " << current_state << " -> " << libcall << " -> ";
            if (is_state_transition_valid(libcall)) {
                // std::cout << "Valid" << std::endl;
                current_state = transition_to_state(libcall);
                // std::cout << current_state << std::endl;
            } else {
                // std::cout << "Invalid transition" << std::endl;
                test_result = 0;
                break;
            }
        }
        EXPECT_EQ(test_result, test_cases_results[i]) << "Test case failed";
    }

    // Cleanup the graph
    destroy_graph ();
    cleanup_test_data();
}

INSTANTIATE_TEST_SUITE_P(
    MemGraph,
    MemGraph_ComprehensiveParam,
    testing::Combine (testing::Bool(),testing::Values(
            "tests/test_data/test-case-1.txt", 
            "tests/test_data/test-case-2.txt",
            "tests/test_data/test-case-3.txt"
            ))
);

