//
// Created by cheyulin on 2/24/16.
//

#include "demon_algorithm.h"

namespace yche {

    unique_ptr<Demon::SubGraph> Demon::ExtractEgoMinusEgo(Demon::Vertex ego_vertex) {
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
            sub_vertex_label_map[sub_vertex][0] = sub_vertex_id_map[sub_vertex];
            sub_vertex_label_map[sub_vertex][1] = 0;
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

    void Demon::DoLabelPropagationOnSingleVertex(unique_ptr<SubGraph> &sub_graph_ptr, SubGraphVertex &sub_graph_Vertex,
                                                 std::mt19937 &rand_generator, const int &last_index_indicator,
                                                 const int &curr_index_indicator,
                                                 property_map<SubGraph, vertex_weight_t>::type sub_vertex_weight_map,
                                                 property_map<SubGraph, vertex_label_t>::type sub_vertex_label_map) {

        auto label_weight_map = map<IndexType, double>();

        //Label Propagation
        for (auto vp_inner = adjacent_vertices(sub_graph_Vertex, *sub_graph_ptr);
             vp_inner.first != vp_inner.second; ++vp_inner.first) {
            auto neighbor_vertex = *vp_inner.first;
            auto neighbor_vertex_label = sub_vertex_label_map[neighbor_vertex][last_index_indicator];
            auto neighbor_vertex_weight = sub_vertex_weight_map[neighbor_vertex];

            auto my_iterator = label_weight_map.find(neighbor_vertex_label);
            if (my_iterator == label_weight_map.end()) {
                label_weight_map.insert(make_pair(neighbor_vertex_label, neighbor_vertex_weight));
            } else {
                label_weight_map[neighbor_vertex_label] += neighbor_vertex_weight;
            }
        }

        //Find Maximum Vote
        auto candidate_label_vec = vector<IndexType>();
        auto max_val = 0.0;
        auto current_vertex = sub_graph_Vertex;
        if (label_weight_map.size() == 0) {
            sub_vertex_label_map[current_vertex][curr_index_indicator] = sub_vertex_label_map[current_vertex][last_index_indicator];
        } else {
            for (auto label_to_weight_pair:label_weight_map) {
                auto label_weight = label_to_weight_pair.second;
                if (label_weight > max_val) {
                    candidate_label_vec.clear();
                    max_val = label_weight;
                }
                if (label_weight >= max_val) {
                    candidate_label_vec.push_back(label_to_weight_pair.first);
                }
            }

            auto choice_index = 0;
            if (candidate_label_vec.size() >= 1) {
                uniform_int_distribution<> distribution(0, candidate_label_vec.size() - 1);
                choice_index = distribution(rand_generator);
            }

            sub_vertex_label_map[current_vertex][curr_index_indicator] = candidate_label_vec[choice_index];
        }
    }

    Demon::CommunityVecPtr Demon::GetCommunitiesBasedOnLabelPropagationResult(
            unique_ptr<SubGraph> &sub_graph_ptr, Vertex &ego_vertex, const int &curr_index_indicator) {
        property_map<SubGraph, vertex_label_t>::type sub_vertex_label_map =
                get(vertex_label, *sub_graph_ptr);
        property_map<SubGraph, vertex_id_t>::type sub_vertex_id_map =
                get(vertex_id, *sub_graph_ptr);
        property_map<Graph, vertex_index_t>::type vertex_index_map =
                get(vertex_index, *graph_ptr_);

        map<int, CommunityPtr> label_indices_map;

        for (auto vp = vertices(*sub_graph_ptr); vp.first != vp.second; ++vp.first) {
            auto sub_vertex = *vp.first;
            auto v_label = sub_vertex_label_map[sub_vertex][curr_index_indicator];
            if (label_indices_map.find(v_label) == label_indices_map.end()) {
                CommunityPtr community = make_unique<vector<IndexType>>();
                label_indices_map.insert(make_pair(v_label, std::move(community)));
            }

            label_indices_map[v_label]->push_back(sub_vertex_id_map[sub_vertex]);
        }

        CommunityVecPtr communities_vec_ptr = make_unique<vector<CommunityPtr>>();
        for (auto iter = label_indices_map.begin(); iter != label_indices_map.end(); ++iter) {
            //Add Ego Vertex
            //Make The Community Vector Sorted
            iter->second->push_back(vertex_index_map[ego_vertex]);
            sort(iter->second->begin(), iter->second->end());
            communities_vec_ptr->push_back(std::move(iter->second));
        }
        if (label_indices_map.size() == 0) {
            //Outlier
            CommunityPtr comm_ptr = make_unique<vector<IndexType>>();
            comm_ptr->push_back(vertex_index_map[ego_vertex]);
            communities_vec_ptr->push_back(std::move(comm_ptr));
        }
        return std::move(communities_vec_ptr);
    }

    Demon::CommunityVecPtr Demon::DoLabelPropagationOnSubGraph
            (unique_ptr<Demon::SubGraph> sub_graph_ptr, Demon::Vertex ego_vertex) {

        int iteration_num = 0;
        property_map<SubGraph, vertex_weight_t>::type sub_vertex_weight_map =
                get(vertex_weight, *sub_graph_ptr);
        property_map<SubGraph, vertex_label_t>::type sub_vertex_label_map =
                get(vertex_label, *sub_graph_ptr);

        while (iteration_num < max_iteration_num_) {
            auto curr_index_indicator = (iteration_num + 1) % 2;
            auto last_index_indicator = iteration_num % 2;
            vector<SubGraphVertex> all_sub_vertices;
            for (auto vp = vertices(*sub_graph_ptr); vp.first != vp.second; ++vp.first) {
                all_sub_vertices.push_back(*vp.first);
            }

            static thread_local random_device rand_d;
            static thread_local std::mt19937 rand_generator(rand_d());
            shuffle(all_sub_vertices.begin(), all_sub_vertices.end(), rand_generator);
            //Each V Do One Propagation
            for (auto vertex_iter = all_sub_vertices.begin(); vertex_iter != all_sub_vertices.end(); ++vertex_iter) {
                //Label Propagation
                DoLabelPropagationOnSingleVertex(sub_graph_ptr, *vertex_iter, rand_generator, last_index_indicator,
                                                 curr_index_indicator, sub_vertex_weight_map, sub_vertex_label_map);
            }

            iteration_num++;
        }
        auto curr_index_indicator = (iteration_num + 1) % 2;

        return std::move(
                GetCommunitiesBasedOnLabelPropagationResult(sub_graph_ptr, ego_vertex, curr_index_indicator));
    }

    double Demon::GetTwoCommunitiesCoverRate(CommunityPtr &left_community, CommunityPtr &right_community) {
        //Sort Merge Join
        //Please Sort Before Call This Function
        vector<IndexType> intersect_set(left_community->size() + right_community->size());
        auto iter_end = set_intersection(left_community->begin(), left_community->end(), right_community->begin(),
                                         right_community->end(), intersect_set.begin());
        intersect_set.resize(iter_end - intersect_set.begin());

        double rate = static_cast<double>(intersect_set.size()) / min(left_community->size(), right_community->size());
        return rate;
    }

    void Demon::MergeTwoCommunitiesToLeftOne(CommunityPtr &left_community, CommunityPtr &right_community) {
        //Executed After GetTwoCommunitiesCoverRate with Sorted left_community & right_community
        vector<IndexType> union_set(left_community->size() + right_community->size());
        auto iter_end = set_union(left_community->begin(), left_community->end(), right_community->begin(),
                                  right_community->end(), union_set.begin());
        union_set.resize(iter_end - union_set.begin());
        left_community = make_unique<vector<IndexType>>(std::move(union_set));
    }

    void Demon::MergeToCommunityCollection(decltype(overlap_community_vec_) &community_collection,
                                           unique_ptr<MergeDataType> &result) {
        //Initial Insert, No Need to Compare
        if (community_collection->size() == 0) {
            for (auto iter_inner = result->begin(); iter_inner != result->end(); ++iter_inner) {
                if ((*iter_inner)->size() > min_community_size_)
                    community_collection->push_back(std::move(*iter_inner));
            }
        } else {
            for (auto iter_inner = result->begin(); iter_inner != result->end(); ++iter_inner) {
                bool first_access_flag = false;
                for (auto iter = community_collection->begin(); iter != community_collection->end(); ++iter) {
                    auto cover_rate_result = GetTwoCommunitiesCoverRate(*iter, *iter_inner);
                    if (cover_rate_result > epsilon_) {
                        MergeTwoCommunitiesToLeftOne(*iter, *iter_inner);
                        break;
                    } else if ((*iter_inner)->size() > min_community_size_ && !first_access_flag) {

                        first_access_flag = true;
                    }
                }
                if (first_access_flag) {
                    community_collection->push_back(std::move(*iter_inner));
                }
            }
        }
    }

    [[deprecated("Replaced With Parallel Execution")]]
    void Demon::ExecuteDaemon() {
        //Clear Former Results
        overlap_community_vec_ = make_unique<vector<CommunityPtr>>();

        for (auto vp = vertices(*graph_ptr_); vp.first != vp.second; ++vp.first) {
            auto ego_vertex = *vp.first;
            auto sub_graph_ptr = ExtractEgoMinusEgo(ego_vertex);
            auto community_vec_ptr = std::move(DoLabelPropagationOnSubGraph(std::move(sub_graph_ptr), ego_vertex));
            MergeToCommunityCollection(overlap_community_vec_, community_vec_ptr);
        }
    }

    unique_ptr<vector<unique_ptr<Demon::BasicDataType>>> Demon::InitBasicComputationData() {
        unique_ptr<vector<unique_ptr<BasicDataType>>> basic_data_vec_ptr = make_unique<vector<unique_ptr<BasicDataType>>>();
        for (auto vp = vertices(*graph_ptr_); vp.first != vp.second; ++vp.first) {
            auto ego_vertex = *vp.first;
            basic_data_vec_ptr->push_back(make_unique<Vertex>(ego_vertex));
        }
        return std::move(basic_data_vec_ptr);
    }

    unique_ptr<Demon::MergeDataType> Demon::LocalComputation(unique_ptr<BasicDataType> seed_member_ptr) {
        auto ego_vertex = *seed_member_ptr;
        auto sub_graph_ptr = ExtractEgoMinusEgo(ego_vertex);
        auto result = std::move(DoLabelPropagationOnSubGraph(std::move(sub_graph_ptr), ego_vertex));
#ifdef DEBUG_DEMON
        for (auto &communtiy_ptr:*result) {
            cout << "Comm:";
            for (auto vertex_id:*communtiy_ptr) {
                cout << vertex_id << ",";
            }
            cout << endl;
        }
#endif
        return std::move(result);
    }

    void Demon::MergeToGlobal(unique_ptr<MergeDataType> &result) {
        MergeToCommunityCollection(overlap_community_vec_, result);
    }

    unique_ptr<Demon::ReduceDataType> Demon::WrapMergeDataToReduceData(unique_ptr<MergeDataType> &merge_data_ptr) {
        return std::move(merge_data_ptr);
    }
}