#include "damon_algorithm.h"
#include "input_output_handler.h"

using namespace yche;

template<typename VertexIndexType>
void ConstructGraphWithEdgeVecForDemon(unique_ptr<Daemon::Graph> &graph_ptr,
                                       map<VertexIndexType, Daemon::Vertex> &name_vertex_map,
                                       map<VertexIndexType, VertexIndexType> &index_name_map,
                                       vector<pair<VertexIndexType, VertexIndexType>> &edges_vec) {
    using namespace boost;
    property_map<Daemon::Graph, vertex_weight_t>::type vertex_weight_map =
            get(vertex_weight, *graph_ptr);
    for (auto iter = edges_vec.begin(); iter != edges_vec.end(); ++iter) {
        if (name_vertex_map.find(iter->first) == name_vertex_map.end()) {
            Daemon::Vertex vertex = add_vertex(*graph_ptr);
            vertex_weight_map[vertex] = 1;
            name_vertex_map.insert(make_pair(iter->first, vertex));
        }
        if (name_vertex_map.find(iter->second) == name_vertex_map.end()) {
            Daemon::Vertex vertex = add_vertex(*graph_ptr);
            vertex_weight_map[vertex] = 1;
            name_vertex_map.insert(make_pair(iter->second, vertex));
        }
        add_edge(name_vertex_map[iter->first], name_vertex_map[iter->second], *graph_ptr);
    }

    property_map<Daemon::Graph, vertex_index_t>::type vertex_index_map = get(vertex_index, *graph_ptr);
    for (auto iter = name_vertex_map.begin(); iter != name_vertex_map.end(); ++iter) {
        index_name_map.insert(make_pair(vertex_index_map[iter->second], iter->first));
    }
}

int main(int argc, char *argv[]) {
    long thread_num = atol(argv[1]);
    char *file_name_ptr = argv[2];
    char *is_reduce_in_merge = argv[3];
    string is_reduce_in_merge_str(is_reduce_in_merge);

    using VertexIndexType =int;
    vector<pair<VertexIndexType, VertexIndexType>> edges_vec;
    ReadEdgeListInToEdgeVector<VertexIndexType>(file_name_ptr, edges_vec);

    auto graph_ptr = make_unique<Daemon::Graph>();
    map<int, Daemon::Vertex> name_vertex_map;
    map<int, int> index_name_map;
    ConstructGraphWithEdgeVecForDemon<VertexIndexType>(graph_ptr, name_vertex_map, index_name_map, edges_vec);

    auto epsilon = 0.5;
    auto min_community_size = 3;
    auto max_iteration = 100;
    auto daemon_ptr = make_unique<Daemon>(epsilon, min_community_size, std::move(graph_ptr),
                                          max_iteration);

    ExecuteAlgorithmWithParallelizer<Daemon, VertexIndexType>(thread_num, is_reduce_in_merge_str, daemon_ptr,
                                                              index_name_map);

//    getchar();

    return 0;
}

