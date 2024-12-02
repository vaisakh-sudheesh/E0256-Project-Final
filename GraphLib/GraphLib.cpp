#include "GraphLib.hpp"

#include <fmt/core.h>
#include <graaflib/graph.h>
#include <graaflib/io/dot.h>
#include <graaflib/algorithm/graph_traversal/depth_first_search.h>


graaf::vertex_id_t LibcCallgraph::add_vertex(const std::string& vertex, bool has_func_call) {
    if (vertex_id_map.find(vertex) != vertex_id_map.end()) {
        //fmt::print("[add_vertex] Vertex {} already exists in the graph \n", vertex);
        return vertex_id_map[vertex];
    }

    graaf::vertex_id_t vertex_id = graph.add_vertex(vertex);
    vertex_id_map[vertex] = vertex_id;
    func_call_map[vertex] = has_func_call;
    return vertex_id;
}

void LibcCallgraph::add_edge(const std::string& vertex_lhs, const std::string& vertex_rhs, const std::string& edge){
    if (vertex_id_map.find(vertex_lhs) == vertex_id_map.end()) {
        //fmt::print("[add_edge] Vertex {} not found in the graph\n", vertex_lhs);
        return;
    }

    if (vertex_id_map.find(vertex_rhs) == vertex_id_map.end()) {
        //fmt::print("[add_edge] Vertex {} not found in the graph\n", vertex_rhs);
        return;
    }

    graph.add_edge(vertex_id_map[vertex_lhs], vertex_id_map[vertex_rhs], edge);
}
void LibcCallgraph::remove_edge(const std::string& vertex_lhs, const std::string& vertex_rhs){
    if (vertex_id_map.find(vertex_lhs) == vertex_id_map.end()) {
        //fmt::print("[remove_edge] Vertex {} not found in the graph\n", vertex_lhs);
        return;
    }

    if (vertex_id_map.find(vertex_rhs) == vertex_id_map.end()) {
        //fmt::print("[remove_edge] Vertex {} not found in the graph\n", vertex_rhs);
        return;
    }

    graph.remove_edge(vertex_id_map[vertex_lhs], vertex_id_map[vertex_rhs]);
}

void LibcCallgraph::remove_vertex(const std::string& vertex) {
    if (vertex_id_map.find(vertex) == vertex_id_map.end()) {
        //fmt::print("Vertex {} not found in the graph\n", vertex);
        return;
    }

    graph.remove_vertex(vertex_id_map[vertex]);
    vertex_id_map.erase(vertex);
    func_call_map.erase(vertex);
}

void LibcCallgraph::combine_vertex(const std::string& vertex_lhs, const std::string& vertex_rhs) {
    if (vertex_id_map.find(vertex_lhs) == vertex_id_map.end()) {
        //fmt::print("[combine_vertex {} {}] Vertex {} not found in the graph\n", vertex_lhs, vertex_rhs, vertex_lhs);
        return;
    }

    if (vertex_id_map.find(vertex_rhs) == vertex_id_map.end()) {
        //fmt::print("[combine_vertex {} {}] Vertex {} not found in the graph\n", vertex_lhs, vertex_rhs, vertex_rhs);
        return;
    }

    // Since combine_vertex does not exist, we need to manually combine edges
    for (const auto& edge : graph.get_edges()) {
        if (edge.first.first == vertex_id_map[vertex_rhs]) {
            graph.add_edge(vertex_id_map[vertex_lhs], edge.first.second, edge.second);
            //fmt::print("[combine_vertex {} {}] Adding edge - lhs: {} -> {}\n", vertex_lhs, vertex_rhs, vertex_lhs, graph.get_vertex(edge.first.second));
        }
        if (edge.first.second == vertex_id_map[vertex_rhs]) {
            if (edge.first.first == vertex_id_map[vertex_lhs]) {
                continue;
            }
            graph.add_edge(edge.first.first, vertex_id_map[vertex_lhs], edge.second);
            //fmt::print("[combine_vertex {} {}] Adding edge - first: {} -> {}\n", vertex_lhs, vertex_rhs, graph.get_vertex(edge.first.first), vertex_lhs);
        }
    }
    graph.remove_vertex(vertex_id_map[vertex_rhs]);
    vertex_id_map.erase(vertex_rhs);
    func_call_map.erase(vertex_rhs);
    //fmt::print("[combine_vertex {} {}] Removing vertex: {}\n",vertex_lhs, vertex_rhs, vertex_rhs);
}

void LibcCallgraph::print(){
    //fmt::print("Graph:\n");
    for (auto& vertex : graph.get_vertices()) {
        //fmt::print("Vertex: {}\n", vertex.second);
        for (auto& edge : graph.get_edges()) {
            //fmt::print("    Edge: {} -> {}\n", edge.first.first, edge.first.second);
        }
    }
}

void LibcCallgraph::dump_todot(const std::string& filename, std::string entry, std::string exit){ 
    const std::filesystem::path path = filename;
  const auto vertex_writer{[this, entry, exit](graaf::vertex_id_t vertex_id,
                                               const std::string& vertex) -> std::string {

    if (vertex == entry) {
        return fmt::format("label=\"{}\",fillcolor=lightgreen, style=filled",
                       vertex);
    } else if (vertex == exit) {
        return fmt::format("label=\"{}\",fillcolor=darkorchid1, style=filled",
                       vertex);
    } else if (func_call_map[vertex]) {
        return fmt::format("label=\"{}\",fillcolor=lightcoral, style=filled",
                       vertex);
    } else {
        return fmt::format("label=\"{}\",fillcolor=lightcyan, style=filled",
                       vertex);
    }
  }};

  const auto edge_writer{[](const graaf::edge_id_t& /*edge_id*/,
                            const auto& edge) -> std::string {
        if (edge == "control") {
            return fmt::format("label=\"\", style=dashed, color=gray, fontcolor=gray");
        } else {
        return fmt::format("label=\"{}\", style=solid, color=red, fontcolor=red",
                       edge);
        }
    }};
        
    graaf::io::to_dot(graph, path, vertex_writer, edge_writer);
}


std::vector<std::string> LibcCallgraph::get_outgoing_edges (const std::string& vertex) {
    std::vector<std::string> outgoing_edges;
    if (vertex_id_map.find(vertex) == vertex_id_map.end()) {
        //fmt::print("[get_outgoing_edges] Vertex {} not found in the graph\n", vertex);
        return outgoing_edges;
    }

    for (const auto& edge : graph.get_neighbors(vertex_id_map[vertex])) {
        outgoing_edges.push_back(graph.get_edge(vertex_id_map[vertex], edge));
    }
    return outgoing_edges;
}

std::vector<std::string> LibcCallgraph::get_neighbors(const std::string& vertex) {
    std::vector<std::string> neighbors;
    if (vertex_id_map.find(vertex) == vertex_id_map.end()) {
        //fmt::print("[get_neighbors] Vertex {} not found in the graph\n", vertex);
        return neighbors;
    }

    for (const auto& neighbor : graph.get_neighbors(vertex_id_map[vertex])) {
        neighbors.push_back(graph.get_vertex(neighbor));
    }
    return neighbors;
}

std::vector<std::string>  LibcCallgraph::get_control_edge_neighbors(const std::string& vertex) const {
    std::vector<std::string> neighbors;
    if (vertex_id_map.find(vertex) == vertex_id_map.end()) {
        //fmt::print("[get_control_edge_neighbors] Vertex {} not found in the graph\n", vertex);
        return neighbors;
    }

    for (const auto& neighbor : graph.get_neighbors(vertex_id_map.at(vertex))) {
        if (graph.get_edge(vertex_id_map.at(vertex), neighbor) == "control") {
            neighbors.push_back(graph.get_vertex(neighbor));
        }
    }
    return neighbors;
}


std::vector<std::string>  LibcCallgraph::get_user_edge_neighbors(const std::string& vertex) const {
    std::vector<std::string> neighbors;
    if (vertex_id_map.find(vertex) == vertex_id_map.end()) {
        //fmt::print("[get_control_edge_neighbors] Vertex {} not found in the graph\n", vertex);
        return neighbors;
    }

    for (const auto& neighbor : graph.get_neighbors(vertex_id_map.at(vertex))) {
        std::string neighbor2 =  graph.get_edge(vertex_id_map.at(vertex), neighbor);
        if (neighbor2.find("user:") == 0) {
            neighbors.push_back(graph.get_vertex(neighbor));
        }
    }
    return neighbors;
}


std::vector<std::string>  LibcCallgraph::get_vertices() const{
    std::vector<std::string> vertices;
    for (const auto& vertex : graph.get_vertices()) {
        vertices.push_back(vertex.second);
    }
    std::reverse(vertices.begin(), vertices.end());
    return vertices;
}



void  LibcCallgraph::insert_graph(const LibcCallgraph& other, const std::string& entry, const std::string& exit){
    for (const auto& vertex : other.graph.get_vertices()) {
        add_vertex(vertex.second, other.func_call_map.at(vertex.second));
    }

    for (const auto& edge : other.graph.get_edges()) {
        add_edge(other.graph.get_vertex(edge.first.first), other.graph.get_vertex(edge.first.second), edge.second);
    }

}