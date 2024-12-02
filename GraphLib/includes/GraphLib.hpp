#ifndef __LIBCCALLGRAPH_HPP__INCLUDED__
#define __LIBCCALLGRAPH_HPP__INCLUDED__

#include <fmt/core.h>
#include <graaflib/graph.h>
#include <unordered_map>

struct LibcCallgraph {
    graaf::directed_graph<std::string, std::string> graph;
    std::unordered_map<std::string, graaf::vertex_id_t> vertex_id_map;

    graaf::vertex_id_t add_vertex(const std::string& vertex);
    void add_edge(const std::string& vertex_lhs, const std::string& vertex_rhs, const std::string& edge);
    void remove_edge(const std::string& vertex_lhs, const std::string& vertex_rhs);
    void remove_vertex(const std::string& vertex);
    void combine_vertex(const std::string& vertex_lhs, const std::string& vertex_rhs);
    
    void print();

    void dump_todot(const std::string& filename);
};


#endif // __LIBCCALLGRAPH_HPP__INCLUDED__
