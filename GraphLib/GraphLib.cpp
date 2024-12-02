#include "GraphLib.hpp"

#include <fmt/core.h>
#include <graaflib/graph.h>
#include <graaflib/io/dot.h>


graaf::vertex_id_t LibcCallgraph::add_vertex(const std::string& vertex, bool has_func_call) {
    if (vertex_id_map.find(vertex) != vertex_id_map.end()) {
        fmt::print("Vertex {} already exists in the graph \n", vertex);
        return vertex_id_map[vertex];
    }

    graaf::vertex_id_t vertex_id = graph.add_vertex(vertex);
    vertex_id_map[vertex] = vertex_id;
    func_call_map[vertex] = has_func_call;
    return vertex_id;
}

void LibcCallgraph::add_edge(const std::string& vertex_lhs, const std::string& vertex_rhs, const std::string& edge){
    if (vertex_id_map.find(vertex_lhs) == vertex_id_map.end()) {
        fmt::print("Vertex {} not found in the graph\n", vertex_lhs);
        return;
    }

    if (vertex_id_map.find(vertex_rhs) == vertex_id_map.end()) {
        fmt::print("Vertex {} not found in the graph\n", vertex_rhs);
        return;
    }

    graph.add_edge(vertex_id_map[vertex_lhs], vertex_id_map[vertex_rhs], edge);
}
void LibcCallgraph::remove_edge(const std::string& vertex_lhs, const std::string& vertex_rhs){
    if (vertex_id_map.find(vertex_lhs) == vertex_id_map.end()) {
        fmt::print("Vertex {} not found in the graph\n", vertex_lhs);
        return;
    }

    if (vertex_id_map.find(vertex_rhs) == vertex_id_map.end()) {
        fmt::print("Vertex {} not found in the graph\n", vertex_rhs);
        return;
    }

    graph.remove_edge(vertex_id_map[vertex_lhs], vertex_id_map[vertex_rhs]);
}

void LibcCallgraph::remove_vertex(const std::string& vertex) {
    if (vertex_id_map.find(vertex) == vertex_id_map.end()) {
        fmt::print("Vertex {} not found in the graph\n", vertex);
        return;
    }

    graph.remove_vertex(vertex_id_map[vertex]);
    vertex_id_map.erase(vertex);
    func_call_map.erase(vertex);
}

void LibcCallgraph::combine_vertex(const std::string& vertex_lhs, const std::string& vertex_rhs) {
    if (vertex_id_map.find(vertex_lhs) == vertex_id_map.end()) {
        fmt::print("Vertex {} not found in the graph\n", vertex_lhs);
        return;
    }

    if (vertex_id_map.find(vertex_rhs) == vertex_id_map.end()) {
        fmt::print("Vertex {} not found in the graph\n", vertex_rhs);
        return;
    }

    // Since combine_vertex does not exist, we need to manually combine edges
    for (const auto& edge : graph.get_edges()) {
        if (edge.first.first == vertex_id_map[vertex_rhs]) {
            graph.add_edge(vertex_id_map[vertex_lhs], edge.first.second, edge.second);
        }
        if (edge.first.second == vertex_id_map[vertex_rhs]) {
            graph.add_edge(edge.first.first, vertex_id_map[vertex_lhs], edge.second);
        }
    }
    graph.remove_vertex(vertex_id_map[vertex_rhs]);
    vertex_id_map.erase(vertex_rhs);
    func_call_map.erase(vertex_rhs);
}

void LibcCallgraph::print(){
    fmt::print("Graph:\n");
    for (auto& vertex : graph.get_vertices()) {
        fmt::print("Vertex: {}\n", vertex.second);
        for (auto& edge : graph.get_edges()) {
            fmt::print("    Edge: {} -> {}\n", edge.first.first, edge.first.second);
        }
    }
}

void LibcCallgraph::dump_todot(const std::string& filename) {
    const std::filesystem::path path = filename;
  const auto vertex_writer{[this](graaf::vertex_id_t vertex_id,
                                  const std::string& vertex) -> std::string {

    if (func_call_map[vertex]) {
        return fmt::format("label=\"{}\",fillcolor=lightcoral, style=filled",
                       vertex);
    } else {
        return fmt::format("label=\"{}\",fillcolor=lightcyan, style=filled",
                       vertex);
    }
  }};

  const auto edge_writer{[](const graaf::edge_id_t& /*edge_id*/,
                            const auto& edge) -> std::string {
    return fmt::format("label=\"{}\", style=solid, color=gray, fontcolor=gray",
                       edge);
  }};
        
    graaf::io::to_dot(graph, path, vertex_writer, edge_writer);
}
