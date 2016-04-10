//
// Created by cheyulin on 2/24/16.
//

#include "damon_algorithm.h"

namespace yche {
    unique_ptr<Daemon::SubGraph> Daemon::ExtractEgoMinusEgo(Daemon::Vertex ego_vertex) {
        unique_ptr<SubGraph> ego_net_ptr = make_unique<SubGraph>();
        auto origin_sub_index_map = map<int, int>();

        property_map<SubGraph, vertex_id_t>::type sub_vertex_id_map =
                get(vertex_id, *ego_net_ptr);
        property_map<SubGraph, vertex_index_t>::type sub_vertex_index_map =
                get(vertex_index, *ego_net_ptr);
        property_map<SubGraph, vertex_weight_t>::type sub_vertex_weight_map =
                get(vertex_weight, *ego_net_ptr);
        property_map<SubGraph, vertex_label_t>::type sub_vertex_label_map =
                get(vertex_label, *ego_net_ptr);
        property_map<Graph, vertex_index_t>::type vertex_index_map =
                get(vertex_index, *graph_ptr_);
        property_map<Graph, vertex_weight_t>::type vertex_weight_map =
                get(vertex_weight, *graph_ptr_);

        int sub_vertex_index = 0;
        vector<SubGraphVertex> sub_vertices;
        //Add SubVertices
        for (auto vp = adjacent_vertices(ego_vertex, *graph_ptr_); vp.first != vp.second; ++vp.first) {
            //Add Vertex
            graph_traits<SubGraph>::vertex_descriptor sub_vertex = add_vertex(*ego_net_ptr);
            sub_vertices.push_back(sub_vertex);

            //Add Vertex Property
            auto graph_vertex_index = vertex_index_map[*vp.first];
            sub_vertex_id_map[sub_vertex] = graph_vertex_index;
            sub_vertex_weight_map[sub_vertex] = vertex_weight_map[*vp.first];

            array<int, 2> sub_vertex_label_map[sub_vertex];
            sub_vertex_label_map->at(0) = sub_vertex_id_map[sub_vertex];
            sub_vertex_label_map->at(1) = 0;

            //Add Indexing Info
            origin_sub_index_map.insert(make_pair(graph_vertex_index, sub_vertex_index_map[sub_vertex]));
        }

        //Add Edges
        for (auto vp = adjacent_vertices(ego_vertex, *graph_ptr_); vp.first != vp.second; ++vp.first) {
            auto source_sub_vertex_index = origin_sub_index_map[vertex_index_map[*vp.first]];
            auto source_sub_vertex = sub_vertices[source_sub_vertex_index];
            for (auto vp_inner = adjacent_vertices(ego_vertex, *graph_ptr_);
                 vp_inner.first != vp_inner.second; ++vp_inner.first) {
                bool is_edge_exists = edge(*vp.first, *vp_inner.first, *graph_ptr_).second;
                if (is_edge_exists) {
                    auto end_sub_vertex_index = origin_sub_index_map[vertex_index_map[*vp_inner.first]];
                    auto end_vertex_ptr = sub_vertices[end_sub_vertex_index];
                    add_edge(source_sub_vertex, end_vertex_ptr, *ego_net_ptr);
                }
            }
        }
        return std::move(ego_net_ptr);
    }

//    Daemon::OverlappingCommunityVec Daemon::DetectCommunitesViaLabelPropagation
//            (unique_ptr<Daemon::Graph> sub_graph_ptr, Daemon::SubGraphVertex ego_vertex) {
//
//        int iteration_num = 0;
////        auto vertices = sub_graph_ptr->m_vertices;
//
//        property_map<SubGraph, vertex_id_t>::type sub_vertex_id_map =
//                get(vertex_id, *sub_graph_ptr);
//        property_map<SubGraph, vertex_index_t>::type sub_vertex_index_map =
//                get(vertex_index, *sub_graph_ptr);
//        property_map<SubGraph, vertex_weight_t>::type sub_vertex_weight_map =
//                get(vertex_weight, *sub_graph_ptr);
//        property_map<SubGraph, vertex_label_t>::type sub_vertex_label_map =
//                get(vertex_label, *sub_graph_ptr);
//
//        while (iteration_num < max_iteration_num_) {
//            auto curr_index_indicator = (iteration_num + 1) % 2;
//            auto last_index_indicator = iteration_num % 2;
//            for (auto &vertex_descriptor: vertices) {
//
//                auto adjacency_iterator_pair = adjacent_vertices(vertex_descriptor, *graph_ptr_);
//                auto end = adjacency_iterator_pair.second;
//
//                auto label_to_weight_map = map<int, double>();
//                //Label Propagation
//                for (auto begin = adjacency_iterator_pair.first; begin != end; ++begin) {
//                    auto neighbor_vertex_label = get(SubGraphVertexLabel, *begin)[last_index_indicator];
//                    auto neighbor_vertex_weight = get(SubGraphVertexWeight, *begin);
//                    auto my_iterator = label_to_weight_map.find(neighbor_vertex_label);
//                    if (my_iterator == label_to_weight_map.end()) {
//                        label_to_weight_map.insert(make_pair(neighbor_vertex_label, neighbor_vertex_weight));
//                    }
//                    else {
//                        label_to_weight_map[neighbor_vertex_label] += neighbor_vertex_weight;
//                    }
//                }
//                //Find Maximum Vote
//
//                auto candidate_label_vec = vector<int>();
//                auto max_val = 0;
//                for (auto &label_to_weight_pair:label_to_weight_map) {
//                    auto label_weight = label_to_weight_pair.second;
//                    if (label_weight > max_val) {
//                        candidate_label_vec.clear();
//                        max_val = label_weight;
//                    }
//                    if (label_weight >= max_val) {
//                        candidate_label_vec.push_back(label_to_weight_pair.first);
//                    }
//                }
//
//
//            }
//
//            iteration_num++;
//        }
//
//        return std::vector<yche::Daemon::Community>();
//    }
}