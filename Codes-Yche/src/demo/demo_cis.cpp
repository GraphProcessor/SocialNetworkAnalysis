//
// Created by cheyulin on 10/5/16.
//

#include "algorithm/cis_algorithm.h"
#include "parallel_utils/dataflow_scheduler.h"
#include "input_output_handler.h"

using namespace yche;

template<typename VertexIndexType>
void ConstructGraphWithEdgeVecForCIS(unique_ptr<Cis::Graph> &graph_ptr,
                                     map<VertexIndexType, Cis::Vertex> &name_vertex_map,
                                     map<VertexIndexType, VertexIndexType> &index_name_map,
                                     vector<EdgeInfo<VertexIndexType>> &edges_vec) {
    using namespace boost;
    property_map<Cis::Graph, edge_weight_t>::type edge_weight_map =
            get(edge_weight, *graph_ptr);
    for (auto iter = edges_vec.begin(); iter != edges_vec.end(); ++iter) {
        if (name_vertex_map.find(iter->src_index_) == name_vertex_map.end()) {
            Cis::Vertex vertex = add_vertex(*graph_ptr);
            name_vertex_map.insert(make_pair(iter->src_index_, vertex));
        }
        if (name_vertex_map.find(iter->dst_index_) == name_vertex_map.end()) {
            Cis::Vertex vertex = add_vertex(*graph_ptr);
            name_vertex_map.insert(make_pair(iter->dst_index_, vertex));
        }
        Cis::Edge edge;
        bool flag;
        tie(edge, flag) = add_edge(name_vertex_map[iter->src_index_], name_vertex_map[iter->dst_index_], *graph_ptr);
        edge_weight_map[edge] = iter->edge_weight_;
        cout << " src:" << iter->src_index_ << ",dst:" << iter->dst_index_ << ",weight:" << iter->edge_weight_ << endl;
    }

    property_map<Cis::Graph, vertex_index_t>::type vertex_index_map = get(vertex_index, *graph_ptr);
    for (auto iter = name_vertex_map.begin(); iter != name_vertex_map.end(); ++iter) {
        index_name_map.insert(make_pair(vertex_index_map[iter->second], iter->first));
    }
}

int main(int argc, char *argv[]) {
    std::ios_base::sync_with_stdio(false);
    long thread_num = 1;
    char *file_name_ptr = argv[1];

    using VertexIndexType =int;
    vector<EdgeInfo<VertexIndexType>> edges_vec;
    ReadEdgeListWithWeightInToEdgeVector<VertexIndexType>(file_name_ptr, edges_vec);

    auto graph_ptr = make_unique<Cis::Graph>();
    map<VertexIndexType, Cis::Vertex> name_vertex_map;
    map<VertexIndexType, VertexIndexType> index_name_map;

    ConstructGraphWithEdgeVecForCIS<VertexIndexType>(graph_ptr, name_vertex_map, index_name_map, edges_vec);

    auto cis_ptr = make_unique<Cis>(std::move(graph_ptr), 0, index_name_map);
    ExecuteAlgorithmWithParallelizer<Cis, VertexIndexType>(thread_num, cis_ptr, index_name_map);

    return 0;
}