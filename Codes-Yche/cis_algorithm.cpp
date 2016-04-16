//
// Created by cheyulin on 4/15/16.
//

#include "cis_algorithm.h"

namespace yche {
    double Cis::CalDensity(const int &size, const double &w_in, const double &w_out, const double &lambda) {
        if (size < 1) return numeric_limits<double>::min();
        double partA = ((1 - lambda) * (w_in / (w_in + w_out)));
        double partB = (lambda * ((2 * w_in) / (size * (size - 1))));
        if (size == 1) partB = lambda;
        return partA + partB;
    }

    unique_ptr<CommunityInfo> Cis::SplitAndChooseBestConnectedComponent(unique_ptr<CommunityMember> community_ptr) {
        queue<IndexType> frontier;
        vector<unique_ptr<CommunityInfo>> community_info_ptr_vec;
        while (community_ptr->size() > 0) {
            IndexType first_vertex = *community_ptr->begin();
            frontier.push(first_vertex);
            auto community_info_ptr = make_unique<CommunityInfo>(0, 0);
            community_info_ptr->memer_ = make_unique<CommunityMember>();

            //One Connected Component
            while (frontier.size() > 0) {
                property_map<Graph, vertex_index_t>::type vertex_index_map = boost::get(vertex_index, *graph_ptr_);
                property_map<Graph, edge_weight_t>::type edge_weight_map = boost::get(edge_weight, *graph_ptr_);
                Edge edge;
                bool edge_exist_flag;
                auto expand_vertex_index = frontier.front();
                auto expand_vertex = vertices_[expand_vertex_index];

                //Add To CommunityInfo Only At Start of Expansion
                community_ptr->insert(expand_vertex_index);
                for (auto vp = adjacent_vertices(vertices_[expand_vertex_index], *graph_ptr_);
                     vp.first != vp.second; ++vp.first) {
                    auto neighbor_vertex = *vp.first;
                    auto adjacency_vertex_index = vertex_index_map[neighbor_vertex];
                    tie(edge, edge_exist_flag) = boost::edge(expand_vertex, neighbor_vertex, *graph_ptr_);
                    //Do W_in and W_out Computation
                    if (community_ptr->find(adjacency_vertex_index) != community_ptr->end()) {
                        community_info_ptr->w_in_ += edge_weight_map[edge];
                        frontier.push(adjacency_vertex_index);
                    }
                    else {
                        community_info_ptr->w_out_ = edge_weight_map[edge];
                    }
                }

                //Erase Already Expanded Vertex
                community_ptr->erase(expand_vertex_index);
            }

            community_info_ptr_vec.push_back(std::move(community_info_ptr));
        }

        //Sort and Select Best CommuntiyInfo , i.e., With Largest Density
        sort(community_info_ptr_vec.begin(), community_info_ptr_vec.end(),
             [](auto &&left_comm_info_ptr, auto &&right_comm_info_ptr) -> bool {
//            return CalDensity(/)
                 auto left_density = CalDensity((left_comm_info_ptr->member_)->size(), left_comm_info_ptr->w_in_,
                                                left_comm_info_ptr->w_out_, lambda_);;

                 auto right_density = CalDensity((right_comm_info_ptr->member_)->size(), right_comm_info_ptr->w_in_,
                                                 right_comm_info_ptr->w_out_, lambda_);

                 if (left_density != right_density) {
                     return left_density > right_density;
                 }
                 else {
                     return (left_comm_info_ptr->member_)->size() > (right_comm_info_ptr->member_)->size();
                 }
             });

        return std:move(community_info_ptr_vec[0]);
    }

    unique_ptr<CommunityMember> Cis::ExpandSeed(unique_ptr<CommunityMember> seed_member_ptr) {
        return std::unique_ptr<yche::CommunityMember>();
    }


}

































