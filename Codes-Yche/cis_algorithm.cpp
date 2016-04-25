//
// Created by cheyulin on 4/15/16.
//

#include "cis_algorithm.h"

namespace yche {
    double Cis::CalculateDensity(const IndexType &size, const double &w_in, const double &w_out, const double &lambda) {
        if (size < 1) return numeric_limits<double>::min();
        double partA = ((1 - lambda) * (w_in / (w_in + w_out)));
        double partB = (lambda * ((2 * w_in) / (size * (size - 1))));
        if (size == 1) partB = lambda;
        return partA + partB;
    }

    unique_ptr<CommunityInfo> Cis::SplitAndChooseBestConnectedComponent(unique_ptr<CommunityMembers> community_ptr) {
        cout << "Split" << endl;
        queue<IndexType> frontier;
        set<IndexType> mark_set;
        vector<unique_ptr<CommunityInfo>> community_info_ptr_vec;
        while (community_ptr->size() > 0) {
            IndexType first_vertex = *community_ptr->begin();
            //Enqueue
            frontier.push(first_vertex);
            mark_set.insert(first_vertex);

            auto community_info_ptr = make_unique<CommunityInfo>(0, 0);
            community_info_ptr->members_ = make_unique<CommunityMembers>();

            //One Connected Component
            while (frontier.size() > 0) {
                property_map<Graph, vertex_index_t>::type vertex_index_map = boost::get(vertex_index, *graph_ptr_);
                property_map<Graph, edge_weight_t>::type edge_weight_map = boost::get(edge_weight, *graph_ptr_);
                Edge edge;
                bool edge_exist_flag;
                auto expand_vertex_index = frontier.front();
                auto expand_vertex = vertices_[expand_vertex_index];

                //Add To CommunityInfo Only At Start of Expansion
//                community_ptr->erase(expand_vertex_index);
                community_info_ptr->members_->insert(expand_vertex_index);
                for (auto vp = adjacent_vertices(vertices_[expand_vertex_index], *graph_ptr_);
                     vp.first != vp.second; ++vp.first) {
                    auto neighbor_vertex = *vp.first;
                    auto adjacency_vertex_index = vertex_index_map[neighbor_vertex];
                    tie(edge, edge_exist_flag) = boost::edge(expand_vertex, neighbor_vertex, *graph_ptr_);
                    //Do W_in and W_out Computation
                    auto iter = community_ptr->find(adjacency_vertex_index);
                    if (mark_set.find(adjacency_vertex_index) == mark_set.end() && iter != community_ptr->end()) {
                        community_info_ptr->w_in_ += edge_weight_map[edge];
                        //Erase and Enqueuee
                        frontier.push(adjacency_vertex_index);
                        mark_set.insert(adjacency_vertex_index);
                    }
                    else {
                        community_info_ptr->w_out_ = edge_weight_map[edge];
                    }
                }
                community_ptr->erase(expand_vertex_index);
                frontier.pop();
            }

            community_info_ptr_vec.push_back(std::move(community_info_ptr));
        }

        //Sort and Select Best CommuntiyInfo , i.e., With Largest Density
        sort(community_info_ptr_vec.begin(), community_info_ptr_vec.end(),
             [this](auto &&left_comm_info_ptr, auto &&right_comm_info_ptr) -> bool {
//            return CalculateDensity(/)
                 double left_density = this->CalculateDensity((left_comm_info_ptr->members_)->size(),
                                                              left_comm_info_ptr->w_in_,
                                                              left_comm_info_ptr->w_out_, this->lambda_);

                 double right_density = this->CalculateDensity((right_comm_info_ptr->members_)->size(),
                                                               right_comm_info_ptr->w_in_,
                                                               right_comm_info_ptr->w_out_, this->lambda_);

                 if (left_density != right_density) {
                     return left_density > right_density;
                 }
                 else {
                     return (left_comm_info_ptr->members_)->size() > (right_comm_info_ptr->members_)->size();
                 }
             });
        return std::move(community_info_ptr_vec[0]);
    }

    unique_ptr<CommunityMembers> Cis::ExpandSeed(unique_ptr<CommunityMembers> seed_member_ptr) {

        auto community_info = make_unique<CommunityInfo>(0, 0);
        community_info->members_ = make_unique<CommunityMembers>();
        //First: Initialize members and neighbors
        auto comp = [](auto &&left_ptr, auto &&right_ptr) -> bool {
            return left_ptr->member_index_ < right_ptr->member_index_;
        };
        using MemberInfoSet = set<unique_ptr<MemberInfo>, decltype(comp)>;
        MemberInfoSet members(comp);
        MemberInfoSet neighbors(comp);

        CommunityMembers to_computed_neighbors;
        property_map<Graph, vertex_index_t>::type vertex_index_map = boost::get(vertex_index, *graph_ptr_);
        property_map<Graph, edge_weight_t>::type edge_weight_map = boost::get(edge_weight, *graph_ptr_);
        //Tally Members of the seed, calculating individual w_in and w_out
        for (auto &seed_vertex_index :*seed_member_ptr) {
            auto member_info_ptr = make_unique<MemberInfo>(seed_vertex_index);
            community_info->members_->insert(seed_vertex_index);
            Vertex seed_vertex = vertices_[seed_vertex_index];

            for (auto vp = adjacent_vertices(seed_vertex, *graph_ptr_); vp.first != vp.second; ++vp.first) {
                auto neighbor_vertex_index = vertex_index_map[*vp.first];
                auto edge_weight = edge_weight_map[edge(seed_vertex, vertices_[neighbor_vertex_index],
                                                        *graph_ptr_).first];
                if (seed_member_ptr->find(neighbor_vertex_index) != seed_member_ptr->end()) {
                    member_info_ptr->w_in_ += edge_weight;
                    community_info->w_in_ += edge_weight;
                }
                else {
                    member_info_ptr->w_out_ += edge_weight;
                    community_info->w_out_ += edge_weight;
                    to_computed_neighbors.insert(neighbor_vertex_index);
                }
            }
            members.insert(std::move(member_info_ptr));
        }

        //Tally For Neighbors of the seed, calculate w_int and w_out
        for (auto &neighbor_vertex_index :to_computed_neighbors) {
            auto neighbor_info_ptr = make_unique<MemberInfo>(neighbor_vertex_index);
            Vertex neighbor_vertex = vertices_[neighbor_vertex_index];
            for (auto vp = adjacent_vertices(neighbor_vertex, *graph_ptr_); vp.first != vp.second; ++vp.first) {
                auto neighbor_neighbor_vertex_index = vertex_index_map[*vp.first];
                auto edge_weight = edge_weight_map[edge(neighbor_vertex, vertices_[neighbor_neighbor_vertex_index],
                                                        *graph_ptr_).first];
                if (seed_member_ptr->find(neighbor_neighbor_vertex_index) != seed_member_ptr->end()) {
                    neighbor_info_ptr->w_in_ += edge_weight;
                }
                else {
                    neighbor_info_ptr->w_out_ += edge_weight;
                }
            }
            neighbors.insert(std::move(neighbor_info_ptr));
        }

//        for (auto &&members_element_ptr:members) {
//            cout << "mem:" << members_element_ptr->member_index_ << "," << members_element_ptr->w_in_ << "," <<
//            members_element_ptr->w_out_ << endl;
//        }
//
//        for (auto &&members_element_ptr:neighbors) {
//            cout << "neighbors:" << members_element_ptr->member_index_ << "," << members_element_ptr->w_in_ << "," <<
//            members_element_ptr->w_out_ << endl;
//        }

        bool change_flag = true;
        //Do Iterative Scan

        while (change_flag) {
            change_flag = false;


            //Add Neighbor to Check List
            vector<unique_ptr<MemberInfo>> to_check_list;
            for (auto &&neighbor_info_ptr:neighbors) {
                to_check_list.push_back(std::move(make_unique<MemberInfo>(*neighbor_info_ptr)));
            }
            auto degree_cmp = [this](auto &&left_ptr, auto &&right_ptr) -> bool {
                return degree(this->vertices_[left_ptr->member_index_], *this->graph_ptr_) <
                       degree(this->vertices_[right_ptr->member_index_], *this->graph_ptr_);
            };
            sort(to_check_list.begin(), to_check_list.end(), degree_cmp);
            //First For Neighbors Iteration
            for (auto &&neighbor_info_ptr:to_check_list) {
                if (CalculateDensity(community_info->members_->size(), community_info->w_in_, community_info->w_out_,
                                     lambda_)
                    < CalculateDensity(community_info->members_->size() + 1,
                                       community_info->w_in_ + neighbor_info_ptr->w_in_,
                                       community_info->w_out_ + neighbor_info_ptr->w_out_, lambda_)) {
                    //Change Neighbor to Member
                    change_flag = true;
                    community_info->w_in_ += neighbor_info_ptr->w_in_;
                    community_info->w_out_ += neighbor_info_ptr->w_out_;
                    community_info->members_->insert(neighbor_info_ptr->member_index_);
                    neighbors.erase(neighbor_info_ptr);

                    auto check_vertex = vertices_[neighbor_info_ptr->member_index_];
                    members.insert(std::move(neighbor_info_ptr));

                    //Update Member and Neighbor List


                    for (auto vp = adjacent_vertices(check_vertex, *graph_ptr_); vp.first != vp.second; ++vp.first) {
                        auto check_neighbor_vertex = *vp.first;
                        auto check_neighbor_vertex_index = vertex_index_map[check_neighbor_vertex];
                        auto check_neighbor_ptr = make_unique<MemberInfo>(check_neighbor_vertex_index);

                        auto iter = members.find(check_neighbor_ptr);
                        auto edge_weight = edge_weight_map[edge(check_vertex, check_neighbor_vertex,
                                                                *graph_ptr_).first];
                        if (iter != members.end() || (iter = neighbors.find(check_neighbor_ptr)) != neighbors.end()) {
                            //Update Info In Members and Neighbors
                            (*iter)->w_in_ += edge_weight;
                            (*iter)->w_out_ -= edge_weight;
                        }

                        else {
                            //Add New Neighbor
                            auto member_info_ptr = make_unique<MemberInfo>(check_neighbor_vertex_index);
                            for (auto vp_inner = adjacent_vertices(check_neighbor_vertex, *graph_ptr_);
                                 vp_inner.first != vp_inner.second; ++vp_inner.first) {
                                auto neighbor_neighbor_vertex_index = vertex_index_map[*vp_inner.first];
                                edge_weight = edge_weight_map[edge(check_neighbor_vertex,
                                                                   vertices_[neighbor_neighbor_vertex_index],
                                                                   *graph_ptr_).first];
                                if (community_info->members_->find(neighbor_neighbor_vertex_index) !=
                                    community_info->members_->end()) {
                                    member_info_ptr->w_in_ += edge_weight;
                                }
                                else {
                                    member_info_ptr->w_out_ += edge_weight;
                                }
                            }
                            neighbors.insert(std::move(member_info_ptr));
                        }
                    }
                }
            }

            //Add Member to Check List
            to_check_list.clear();
            for (auto &&member_info_ptr:members) {
                to_check_list.push_back(std::move(make_unique<MemberInfo>(*member_info_ptr)));
            }
            sort(to_check_list.begin(), to_check_list.end(), degree_cmp);
            for (auto &&member_info_ptr:to_check_list) {
                if (CalculateDensity(community_info->members_->size(), community_info->w_in_, community_info->w_out_,
                                     lambda_)
                    < CalculateDensity(community_info->members_->size() - 1,
                                       community_info->w_in_ - member_info_ptr->w_in_,
                                       community_info->w_out_ - member_info_ptr->w_out_, lambda_)) {
                    change_flag = true;
                    community_info->w_in_ -= member_info_ptr->w_in_;
                    community_info->w_out_ -= member_info_ptr->w_out_;
                    community_info->members_->erase(member_info_ptr->member_index_);
                    members.erase(member_info_ptr);
                    auto check_vertex = vertices_[member_info_ptr->member_index_];
                    neighbors.insert(std::move(member_info_ptr));

                    //Update Member and Neighbor List

                    for (auto vp = adjacent_vertices(check_vertex, *graph_ptr_); vp.first != vp.second; ++vp.first) {
                        auto check_neighbor_vertex = *vp.first;
                        auto check_neighbor_vertex_index = vertex_index_map[check_neighbor_vertex];
                        auto check_neighbor_ptr = std::move(make_unique<MemberInfo>(check_neighbor_vertex_index));
                        auto iter = members.find(check_neighbor_ptr);
                        auto edge_weight = edge_weight_map[edge(check_vertex, check_neighbor_vertex,
                                                                *graph_ptr_).first];
                        if (iter != members.end() || (iter = neighbors.find(check_neighbor_ptr)) != neighbors.end()) {
                            //Update Info In Members and Neighbors
                            (*iter)->w_in_ -= edge_weight;
                            (*iter)->w_out_ += edge_weight;
                        }
                    }
                }
            }

            community_info = std::move(SplitAndChooseBestConnectedComponent(std::move(community_info->members_)));
        }
        return std::move(community_info->members_);
    }


    double Cis::GetTwoCommunitiesCoverRate(unique_ptr<CommunityMembers> &&left_community,
                                           unique_ptr<CommunityMembers> &&right_community) {
        vector<IndexType> intersect_set(left_community->size() + right_community->size());
        auto iter_end = set_intersection(left_community->begin(), left_community->end(), right_community->begin(),
                                         right_community->end(), intersect_set.begin());
        intersect_set.resize(iter_end - intersect_set.begin());
        //left_community->size() call before std::move it
        double rate = static_cast<double>(intersect_set.size()) / min(left_community->size(), right_community->size());
        return rate;
    }

    unique_ptr<CommunityMembers> Cis::MergeTwoCommunities(unique_ptr<CommunityMembers> &&left_community,
                                                          unique_ptr<CommunityMembers> &&right_community) {

        vector<IndexType> union_set(left_community->size() + right_community->size());
        auto iter_end = set_union(left_community->begin(), left_community->end(), right_community->begin(),
                                  right_community->end(), union_set.begin());
        union_set.resize(iter_end - union_set.begin());
        unique_ptr<CommunityMembers> union_community = make_unique<set<IndexType>>();
        for (auto iter = union_set.begin(); iter != union_set.end(); ++iter) {
            union_community->insert(*iter);
        }

        return std::move(union_community);
    }

    unique_ptr<Cis::CommunityVec> Cis::ExecuteCis() {
        auto overlapping_communities_ptr = make_unique<CommunityVec>();

        for (auto vp = vertices(*graph_ptr_); vp.first != vp.second; ++vp.first) {
            //First
            property_map<Graph, vertex_index_t>::type vertex_index_map = boost::get(vertex_index, *graph_ptr_);
            Vertex vertex = *vp.first;
            auto partial_comm_members = make_unique<CommunityMembers>();
            partial_comm_members->insert(vertex_index_map[vertex]);
            auto result_community = std::move(ExpandSeed(std::move(partial_comm_members)));


            //Second
            cout << vertex_name_map_[vertex_index_map[vertex]];
            cout << "  size:\t" << result_community->size() << ":\t";
            for (auto index:*result_community) {
                cout << vertex_name_map_[index] << ",";
            }
            cout << endl;
            if (overlapping_communities_ptr->size() == 0) {
                overlapping_communities_ptr->push_back(std::move(result_community));
            }
            else {
                bool insert_flag = true;
                for (auto &&comm_ptr:*overlapping_communities_ptr) {
                    auto cover_rate = GetTwoCommunitiesCoverRate(std::move(comm_ptr), std::move(result_community));
                    if (cover_rate > 1 - DOUBLE_ACCURACY) {
                        comm_ptr = MergeTwoCommunities(std::move(comm_ptr), std::move(result_community));
                        insert_flag = false;
                        break;

                    }
                }
                if (insert_flag) {
                    overlapping_communities_ptr->push_back(std::move(result_community));
                }
            }
        }
        return std::move(overlapping_communities_ptr);
    }

//    unique_ptr<CommunityMembers> Cis::LocalComputation(unique_ptr<CommunityMembers> seed_member_ptr) {
//        return std::unique_ptr<yche::CommunityMembers>();
//    }
//
//    void Cis::MergeToGlobal(unique_ptr<CommunityMembers> &&result) {
//
//    }
//
//    unique_ptr<vector<BasicData>> Cis::InitBasicComputationData() {
//        auto data_vec_ptr = make_unique<vector<BasicData>>(vertices_.size());
//
////        return std::unique_ptr<vector<yche::Cis::BasicData, allocator<yche::Cis::BasicData>>>();
//
//        for (auto vp = vertices(*graph_ptr_); vp.first != vp.second; ++vp.first) {
//            //First
//            property_map<Graph, vertex_index_t>::type vertex_index_map = boost::get(vertex_index, *graph_ptr_);
//            Vertex vertex = *vp.first;
//            auto partial_comm_members = make_unique<CommunityMembers>();
//            partial_comm_members->insert(vertex_index_map[vertex]);
//        }
//    }


}



































