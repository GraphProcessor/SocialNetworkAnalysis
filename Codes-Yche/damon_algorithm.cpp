//
// Created by cheyulin on 2/24/16.
//

#include "damon_algorithm.h"

namespace yche {
    unique_ptr<Daemon::SubGraph> Daemon::ExtractEgoMinusEgo(Daemon::VertexDescriptor ego_vertex) {
        unique_ptr<SubGraph> ego_net_ptr = make_unique<SubGraph>();
        auto adjacency_iterator_pair = adjacent_vertices(ego_vertex, graph_);
        auto origin_index_to_sub_index_map = map<VertexIndexType, VertexIndexType>();

        auto end = adjacency_iterator_pair.second;
        int sub_index = 0;
        property_map<SubGraph, vertex_id_t>::type SubGraphVertexId =
                get(vertex_id_t(), *ego_net_ptr);
        property_map<SubGraph, vertex_weight_t>::type SubGraphVertexWeight =
                get(vertex_weight_t(), *ego_net_ptr);
        property_map<SubGraph, vertex_label_t>::type SubGraphVertexLabel =
                get(vertex_label_t(), *ego_net_ptr);
        property_map<Graph, vertex_index_t>::type GraphVertexIndex =
                get(vertex_index_t(), graph_);
        property_map<Graph, vertex_weight_t>::type GraphVertexWeight =
                get(vertex_weight_t(), graph_);

        for (auto begin = adjacency_iterator_pair.first; begin != end; ++begin) {
            //Add VertexDescriptor
            SubGraphVertexDescriptor vertex = add_vertex(*ego_net_ptr);

            //Add VertexDescriptor Property
            VertexIndexType graph_vertex_index = get(GraphVertexIndex, *begin);
            put(SubGraphVertexId, vertex, graph_vertex_index);
            put(SubGraphVertexWeight, vertex, get(GraphVertexWeight, *begin));

            array<VertexIndexType, 2> label_array;
            label_array[0] = graph_vertex_index;
            label_array[1] = 0;
            put(SubGraphVertexLabel, vertex, label_array);

            //Add Indexing Info
            origin_index_to_sub_index_map.insert(make_pair(graph_vertex_index, sub_index));
            sub_index++;
        }

        //Add Edges
        for (auto begin = adjacency_iterator_pair.first; begin != end; ++begin) {
            auto start_vertex_id = origin_index_to_sub_index_map[get(GraphVertexIndex, *begin)];
            auto start_vertex_ptr = ego_net_ptr->m_vertices[start_vertex_id];
            for (auto begin_check_vertex = adjacency_iterator_pair.first;
                 begin_check_vertex != end; ++begin_check_vertex) {
                bool is_edge_exists = edge(*begin, *begin_check_vertex, graph_).second;
                if (is_edge_exists) {
                    auto end_vertex_id = origin_index_to_sub_index_map[get(GraphVertexIndex, *begin_check_vertex)];
                    auto end_vertex_ptr = ego_net_ptr->m_vertices[end_vertex_id];
                    add_edge(start_vertex_ptr, end_vertex_ptr, ego_net_ptr);
                }
            }
        }

        return std::move(ego_net_ptr);
    }

    Daemon::OverlappingCommunityVec Daemon::DetectCommunitesViaLabelPropagation
            (unique_ptr<Daemon::Graph> sub_graph, Daemon::SubGraphVertexDescriptor ego_vertex) {
        int iteration_num = 0;
        auto vertices = sub_graph->m_vertices;

        property_map<SubGraph, vertex_id_t>::type SubGraphVertexId =
                get(vertex_id_t(), *sub_graph);
        property_map<SubGraph, vertex_weight_t>::type SubGraphVertexWeight =
                get(vertex_weight_t(), *sub_graph);
        property_map<SubGraph, vertex_label_t>::type SubGraphVertexLabel =
                get(vertex_label_t(), *sub_graph);

        while (iteration_num < max_iteration_num_) {
            auto curr_index_indicator = (iteration_num + 1) % 2;
            auto last_index_indicator = iteration_num % 2;
            for (auto &vertex_descriptor: vertices) {

                auto adjacency_iterator_pair = adjacent_vertices(vertex_descriptor, graph_);
                auto end = adjacency_iterator_pair.second;

                auto label_to_weight_map = map<VertexIndexType, double>();
                //Label Propagation
                for (auto begin = adjacency_iterator_pair.first; begin != end; ++begin) {
                    auto neighbor_vertex_label = get(SubGraphVertexLabel, *begin)[last_index_indicator];
                    auto neighbor_vertex_weight = get(SubGraphVertexWeight, *begin);
                    auto my_iterator = label_to_weight_map.find(neighbor_vertex_label);
                    if (my_iterator == label_to_weight_map.end()) {
                        label_to_weight_map.insert(make_pair(neighbor_vertex_label, neighbor_vertex_weight));
                    }
                    else {
                        label_to_weight_map[neighbor_vertex_label] += neighbor_vertex_weight;
                    }
                }
                //Find Maximum Vote

                auto candidate_label_vec = vector<VertexIndexType>();
                auto max_val = 0;
                for (auto &label_to_weight_pair:label_to_weight_map) {
                    auto label_weight = label_to_weight_pair.second;
                    if (label_weight > max_val) {
                        candidate_label_vec.clear();
                        max_val = label_weight;
                    }
                    if (label_weight >= max_val) {
                        candidate_label_vec.push_back(label_to_weight_pair.first);
                    }
                }


            }

            iteration_num++;
        }

        return std::vector<yche::Daemon::Community>();
    }
}