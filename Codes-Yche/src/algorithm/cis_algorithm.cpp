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

    double Cis::CalculateDensity(unique_ptr<CommunityInfo> &community_info_ptr) {
        return CalculateDensity((community_info_ptr->members_)->size(),
                                community_info_ptr->w_in_,
                                community_info_ptr->w_out_, this->lambda_);
    }

    double Cis::CalculateDensity(unique_ptr<CommunityInfo> &community_info_ptr,
                                 unique_ptr<MemberInfo> &member_info_ptr, const MutationType &mutation_type) {
        if (mutation_type == MutationType::add_neighbor)
            return CalculateDensity((community_info_ptr->members_)->size() + 1,
                                    community_info_ptr->w_in_ + member_info_ptr->w_in_,
                                    community_info_ptr->w_out_ + member_info_ptr->w_out_, this->lambda_);
        else
            return CalculateDensity((community_info_ptr->members_)->size() - 1,
                                    community_info_ptr->w_in_ - member_info_ptr->w_in_,
                                    community_info_ptr->w_out_ - member_info_ptr->w_out_, this->lambda_);

    }

    unique_ptr<CommunityInfo> Cis::SplitAndChooseBestConnectedComponent(unique_ptr<CommunityMemberSet> &community_ptr) {
        queue<IndexType> frontier;
        std::unordered_set<IndexType> mark_set;
        vector<unique_ptr<CommunityInfo>> community_info_ptr_vec;
        while (community_ptr->size() > 0) {
            IndexType first_vertex = *community_ptr->begin();
            //Enqueue
            frontier.push(first_vertex);
            mark_set.insert(first_vertex);

            auto community_info_ptr = make_unique<CommunityInfo>(0, 0);
            community_info_ptr->members_ = make_unique<CommunityMemberSet>();

            //One Connected Component
            while (frontier.size() > 0) {
                property_map<Graph, vertex_index_t>::type vertex_index_map = boost::get(vertex_index, *graph_ptr_);
                property_map<Graph, edge_weight_t>::type edge_weight_map = boost::get(edge_weight, *graph_ptr_);
                Edge edge;
                bool edge_exist_flag;
                auto expand_vertex_index = frontier.front();
                auto expand_vertex = vertices_[expand_vertex_index];

                //Add To CommunityInfo Only At Start of Expansion
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
                        //Erase and Enqueue
                        frontier.push(adjacency_vertex_index);
                        mark_set.insert(adjacency_vertex_index);
                    } else {
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
             [this](auto &left_comm_info_ptr, auto &right_comm_info_ptr) -> bool {
                 double left_density = this->CalculateDensity(left_comm_info_ptr);
                 double right_density = this->CalculateDensity(right_comm_info_ptr);
                 if (left_density != right_density) {
                     return left_density > right_density;
                 } else {
                     return (left_comm_info_ptr->members_)->size() > (right_comm_info_ptr->members_)->size();
                 }
             });
        return std::move(community_info_ptr_vec[0]);
    }

    void Cis::InitializationForSeedExpansion(const unique_ptr<CommunityMemberSet> &seed_member_ptr,
                                             unique_ptr<CommunityInfo> &community_info_ptr, MemberInfoMap &members,
                                             MemberInfoMap &neighbors, CommunityMemberSet &to_computed_neighbors,
                                             property_map<Graph, vertex_index_t>::type &vertex_index_map,
                                             property_map<Graph, edge_weight_t>::type &edge_weight_map) {
        //Tally Members of the seed, calculating individual w_in and w_out
        for (auto &seed_vertex_index :*seed_member_ptr) {
            auto member_info_ptr = make_unique<MemberInfo>(seed_vertex_index);
            community_info_ptr->members_->insert(seed_vertex_index);
            Vertex seed_vertex = vertices_[seed_vertex_index];

            for (auto vp = adjacent_vertices(seed_vertex, *graph_ptr_); vp.first != vp.second; ++vp.first) {
                auto neighbor_vertex_index = vertex_index_map[*vp.first];
                auto edge_weight = edge_weight_map[edge(seed_vertex, vertices_[neighbor_vertex_index],
                                                        *graph_ptr_).first];
                if (seed_member_ptr->find(neighbor_vertex_index) != seed_member_ptr->end()) {
                    member_info_ptr->w_in_ += edge_weight;
                    community_info_ptr->w_in_ += edge_weight;
                } else {
                    member_info_ptr->w_out_ += edge_weight;
                    community_info_ptr->w_out_ += edge_weight;
                    to_computed_neighbors.insert(neighbor_vertex_index);
                }
            }
            members.insert(make_pair(member_info_ptr->member_index_, std::move(member_info_ptr)));
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
                } else {
                    neighbor_info_ptr->w_out_ += edge_weight;
                }
            }
            neighbors.insert(make_pair(neighbor_info_ptr->member_index_, std::move(neighbor_info_ptr)));
        }
    }

    void Cis::UpdateMembersNeighborsCommunityInfo(const Cis::Vertex &mutate_vertex,
                                                  unique_ptr<CommunityInfo> &community_info_ptr,
                                                  MemberInfoMap &members,
                                                  MemberInfoMap &neighbors, const MutationType &mutation_type,
                                                  property_map<Graph, vertex_index_t>::type &vertex_index_map,
                                                  property_map<Graph, edge_weight_t>::type &edge_weight_map) {
        if (mutation_type == MutationType::add_neighbor) {
            UpdateMembersNeighborsCommunityInfoForAddNeighbor(mutate_vertex, community_info_ptr, members,
                                                              neighbors, vertex_index_map, edge_weight_map);
        } else {
            UpdateMembersNeighborsCommunityInfoForRemoveMember(mutate_vertex, community_info_ptr, members,
                                                               neighbors, vertex_index_map, edge_weight_map);
        }
    }

    void Cis::UpdateMembersNeighborsCommunityInfoForAddNeighbor(const Cis::Vertex &mutate_vertex,
                                                                unique_ptr<CommunityInfo> &community_info_ptr,
                                                                MemberInfoMap &members, MemberInfoMap &neighbors,
                                                                property_map<Graph, vertex_index_t>::type &vertex_index_map,
                                                                property_map<Graph, edge_weight_t>::type &edge_weight_map) {
        //Update Member and Neighbor List
        for (auto vp = adjacent_vertices(mutate_vertex, *graph_ptr_); vp.first != vp.second; ++vp.first) {
            auto check_neighbor_vertex = *vp.first;
            auto check_neighbor_vertex_index = vertex_index_map[check_neighbor_vertex];
            auto check_neighbor_ptr = make_unique<MemberInfo>(check_neighbor_vertex_index);
            auto edge_weight = edge_weight_map[edge(mutate_vertex, check_neighbor_vertex,
                                                    *graph_ptr_).first];

            auto iter = members.find(check_neighbor_ptr->member_index_);
            if (iter != members.end() ||
                (iter = neighbors.find(check_neighbor_ptr->member_index_)) != neighbors.end()) {
                //Update Info In Members and Neighbors
                iter->second->w_in_ += edge_weight;
                iter->second->w_out_ -= edge_weight;
            } else {
                //Add New Neighbor
                auto member_info_ptr = make_unique<MemberInfo>(check_neighbor_vertex_index);
                for (auto vp_inner = adjacent_vertices(check_neighbor_vertex, *graph_ptr_);
                     vp_inner.first != vp_inner.second; ++vp_inner.first) {
                    auto neighbor_neighbor_vertex_index = vertex_index_map[*vp_inner.first];
                    edge_weight = edge_weight_map[edge(check_neighbor_vertex,
                                                       vertices_[neighbor_neighbor_vertex_index],
                                                       *graph_ptr_).first];
                    if (community_info_ptr->members_->find(neighbor_neighbor_vertex_index) !=
                        community_info_ptr->members_->end()) {
                        member_info_ptr->w_in_ += edge_weight;
                    } else {
                        member_info_ptr->w_out_ += edge_weight;
                    }
                }
                neighbors.insert(make_pair(member_info_ptr->member_index_, std::move(member_info_ptr)));
            }
        }
    }

    void Cis::UpdateMembersNeighborsCommunityInfoForRemoveMember(const Cis::Vertex &mutate_vertex,
                                                                 unique_ptr<CommunityInfo> &community_info_ptr,
                                                                 MemberInfoMap &members, MemberInfoMap &neighbors,
                                                                 property_map<Graph, vertex_index_t>::type &vertex_index_map,
                                                                 property_map<Graph, edge_weight_t>::type &edge_weight_map) {
        //Update Member and Neighbor List
        for (auto vp = adjacent_vertices(mutate_vertex, *graph_ptr_); vp.first != vp.second; ++vp.first) {
            auto check_neighbor_vertex = *vp.first;
            auto check_neighbor_vertex_index = vertex_index_map[check_neighbor_vertex];
            auto check_neighbor_ptr = std::move(make_unique<MemberInfo>(check_neighbor_vertex_index));
            auto edge_weight = edge_weight_map[edge(mutate_vertex, check_neighbor_vertex,
                                                    *graph_ptr_).first];

            auto iter = members.find(check_neighbor_ptr->member_index_);
            if (iter != members.end() ||
                (iter = neighbors.find(check_neighbor_ptr->member_index_)) != neighbors.end()) {
                //Update Info In Members and Neighbors
                iter->second->w_in_ -= edge_weight;
                iter->second->w_out_ += edge_weight;
            }
        }

    }

    unique_ptr<CommunityMemberVec> Cis::ExpandSeed(unique_ptr<CommunityMemberSet> &seed_member_ptr) {
        //First: Initialize members and neighbors(or frontier)
        auto community_info_ptr = make_unique<CommunityInfo>(0, 0);
        community_info_ptr->members_ = make_unique<CommunityMemberSet>();
        MemberInfoMap members;
        MemberInfoMap neighbors;
        CommunityMemberSet to_computed_neighbors;
        property_map<Graph, vertex_index_t>::type vertex_index_map = boost::get(vertex_index, *graph_ptr_);
        property_map<Graph, edge_weight_t>::type edge_weight_map = boost::get(edge_weight, *graph_ptr_);

        InitializationForSeedExpansion(seed_member_ptr, community_info_ptr, members, neighbors, to_computed_neighbors,
                                       vertex_index_map, edge_weight_map);

        bool change_flag = true;
        //Do Iterative Scan, Until Local-Density-Optimal
        while (change_flag) {
            change_flag = false;

            //First Init: Add Neighbor to Check List
            vector<unique_ptr<MemberInfo>> to_check_list;
            for (auto &neighbor_info_ptr:neighbors) {
                to_check_list.push_back(std::move(make_unique<MemberInfo>(*neighbor_info_ptr.second)));
            }
            auto degree_cmp = [this](auto &left_ptr, auto &right_ptr) -> bool {
                return degree(this->vertices_[left_ptr->member_index_], *this->graph_ptr_) <
                       degree(this->vertices_[right_ptr->member_index_], *this->graph_ptr_);
            };
            sort(to_check_list.begin(), to_check_list.end(), degree_cmp);
            //First For Add-Neighbor Iteration, Check all the neighbors in the frontier
            for (auto &neighbor_info_ptr:to_check_list) {
                if (CalculateDensity(community_info_ptr)
                    < CalculateDensity(community_info_ptr, neighbor_info_ptr, MutationType::add_neighbor)) {
                    //Add Neighbor
                    change_flag = true;
                    community_info_ptr->UpdateInfoForMutation(*neighbor_info_ptr, MutationType::add_neighbor);
                    neighbors.erase(neighbor_info_ptr->member_index_);
                    auto check_vertex = vertices_[neighbor_info_ptr->member_index_];
                    members.insert(make_pair(neighbor_info_ptr->member_index_, std::move(neighbor_info_ptr)));
                    //Update Member and Neighbor List
                    UpdateMembersNeighborsCommunityInfo(check_vertex, community_info_ptr, members,
                                                        neighbors, MutationType::add_neighbor,
                                                        vertex_index_map, edge_weight_map);
                }
            }

            //Second Init: Add Member to Check List
            to_check_list.clear();
            for (auto &member_info_ptr:members) {
                to_check_list.push_back(std::move(make_unique<MemberInfo>(*member_info_ptr.second)));
            }
            sort(to_check_list.begin(), to_check_list.end(), degree_cmp);
            //Second For Remove-Member Iteration, check all the members in the current community
            for (auto &member_info_ptr:to_check_list) {
                if (CalculateDensity(community_info_ptr)
                    < CalculateDensity(community_info_ptr, member_info_ptr, MutationType::remove_member)) {
                    //Remove Member
                    change_flag = true;
                    community_info_ptr->UpdateInfoForMutation(*member_info_ptr, MutationType::add_neighbor);
                    members.erase(member_info_ptr->member_index_);
                    auto check_vertex = vertices_[member_info_ptr->member_index_];
                    neighbors.insert(make_pair(member_info_ptr->member_index_, std::move(member_info_ptr)));
                    //Update Member and Neighbor List
                    UpdateMembersNeighborsCommunityInfo(check_vertex, community_info_ptr, members,
                                                        neighbors, MutationType::remove_member,
                                                        vertex_index_map, edge_weight_map);
                }
            }
//            following line commented because the process make the community connected
//            community_info_ptr = std::move(SplitAndChooseBestConnectedComponent(community_info_ptr->members_));
        }
        CommunityMemberVec community_member_vector;
        community_member_vector.resize(community_info_ptr->members_->size());
        auto local_index = 0;
        for (auto &member_index:*community_info_ptr->members_) {
            community_member_vector[local_index] = member_index;
            local_index++;
        }

        //For Later Sort-Merge-Join
        sort(community_member_vector.begin(), community_member_vector.end());
        return std::move(make_unique<CommunityMemberVec>(std::move(community_member_vector)));
    }

    double Cis::GetTwoCommunitiesCoverRate(unique_ptr<CommunityMemberVec> &left_community,
                                           unique_ptr<CommunityMemberVec> &right_community) {
        vector<IndexType> intersect_set(left_community->size() + right_community->size());
        auto iter_end = set_intersection(left_community->begin(), left_community->end(), right_community->begin(),
                                         right_community->end(), intersect_set.begin());
        intersect_set.resize(iter_end - intersect_set.begin());

        double rate = static_cast<double>(intersect_set.size()) / min(left_community->size(), right_community->size());
        return rate;
    }

    unique_ptr<CommunityMemberVec> Cis::MergeTwoCommunities(unique_ptr<CommunityMemberVec> &left_community,
                                                            unique_ptr<CommunityMemberVec> &right_community) {
        vector<IndexType> union_set(left_community->size() + right_community->size());
        auto iter_end = set_union(left_community->begin(), left_community->end(), right_community->begin(),
                                  right_community->end(), union_set.begin());
        union_set.resize(iter_end - union_set.begin());
        return std::move(make_unique<CommunityMemberVec>(std::move(union_set)));
    }

    void Cis::MergeToCommunityCollection(decltype(overlap_community_vec_) &community_collection,
                                         unique_ptr<MergeDataType> &result) {
        if (community_collection->size() == 0) {
            community_collection->push_back(std::move(result));
        } else {
            bool insert_flag = true;
            for (auto &comm_ptr:*community_collection) {
                auto cover_rate = GetTwoCommunitiesCoverRate(comm_ptr, result);
                if (cover_rate > 1 - DOUBLE_ACCURACY) {
                    comm_ptr = MergeTwoCommunities(comm_ptr, result);
                    insert_flag = false;
                    break;
                }
            }
            if (insert_flag) {
                community_collection->push_back(std::move(result));
            }
        }
    }

    [[deprecated("Replaced With Parallel Execution")]]
    unique_ptr<Cis::OverlappingCommunityVec> Cis::ExecuteCis() {
        auto overlapping_communities_ptr = make_unique<OverlappingCommunityVec>();

        for (auto vp = vertices(*graph_ptr_); vp.first != vp.second; ++vp.first) {
            //First
            property_map<Graph, vertex_index_t>::type vertex_index_map = boost::get(vertex_index, *graph_ptr_);
            Vertex vertex = *vp.first;
            auto partial_comm_members = make_unique<CommunityMemberSet>();
            partial_comm_members->insert(vertex_index_map[vertex]);
            auto result_community = std::move(ExpandSeed(partial_comm_members));
            //Second
            MergeToCommunityCollection(overlap_community_vec_, result_community);
        }
        return std::move(overlapping_communities_ptr);
    }

    unique_ptr<vector<unique_ptr<Cis::BasicDataType>>> Cis::InitBasicComputationData() {
        auto basic_data_vec_ptr = make_unique<vector<unique_ptr<BasicDataType>>>();
        for (auto vp = vertices(*graph_ptr_); vp.first != vp.second; ++vp.first) {
            property_map<Graph, vertex_index_t>::type vertex_index_map = boost::get(vertex_index, *graph_ptr_);
            Vertex vertex = *vp.first;
            auto partial_comm_members = make_unique<CommunityMemberSet>();
            partial_comm_members->insert(vertex_index_map[vertex]);
            basic_data_vec_ptr->push_back(std::move(partial_comm_members));
        }
        return std::move(basic_data_vec_ptr);
    }

    unique_ptr<CommunityMemberVec> Cis::LocalComputation(unique_ptr<BasicDataType> seed_member_ptr) {
        auto result_community = std::move(ExpandSeed(seed_member_ptr));
        return result_community;
    }

    void Cis::MergeToGlobal(unique_ptr<MergeDataType> &result) {
        MergeToCommunityCollection(overlap_community_vec_, result);
    }

    unique_ptr<Cis::ReduceDataType> Cis::WrapMergeDataToReduceData(unique_ptr<MergeDataType> &merge_data_ptr) {
        auto reduce_data_ptr = make_unique<ReduceDataType>();
        reduce_data_ptr->push_back(std::move(merge_data_ptr));
        return std::move(reduce_data_ptr);
    }
}
