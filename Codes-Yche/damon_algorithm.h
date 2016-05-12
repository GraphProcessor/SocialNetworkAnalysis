//
// Created by cheyulin on 2/24/16.
//

#ifndef CODES_YCHE_DAMON_ALGORITHM_H
#define CODES_YCHE_DAMON_ALGORITHM_H

#include "include_header.h"

using namespace boost;
using namespace std;

//Add User-Defined Tags
enum vertex_weight_t {
    vertex_weight
};
enum vertex_label_t {
    vertex_label
};

enum vertex_id_t {
    vertex_id
};

namespace boost {
    BOOST_INSTALL_PROPERTY(vertex, weight);
    BOOST_INSTALL_PROPERTY(vertex, label);
    BOOST_INSTALL_PROPERTY(vertex, id);
}

namespace yche {
    class Daemon {
    public:
        using VertexProperties = property<vertex_weight_t, double,
                property<vertex_index_t, unsigned long>>;
        //Edge Vertex
        using Graph = adjacency_list<setS, vecS, undirectedS, VertexProperties>;

        using SubGraphVertexProperties = property<vertex_weight_t, double,
                property<vertex_id_t, unsigned long,
                        property<vertex_label_t, array<unsigned long, 2>>>>;

        using SubGraph = adjacency_list<setS, vecS, undirectedS, SubGraphVertexProperties>;

        using Vertex = graph_traits<Graph>::vertex_descriptor;
        using SubGraphVertex = graph_traits<SubGraph>::vertex_descriptor;
        using CommunityPtr = unique_ptr<set<unsigned long>>;
        using CommunityVecPtr = unique_ptr<vector<CommunityPtr>>;

        //Start Implementation Interfaces For Parallelizer Traits
        CommunityVecPtr overlap_community_vec_;
        using BasicData = Vertex;
        using MergeData = vector<CommunityPtr>;

        unique_ptr<vector<unique_ptr<BasicData>>> InitBasicComputationData();

        unique_ptr<MergeData> LocalComputation(unique_ptr<BasicData> seed_member_ptr);

        void MergeToGlobal(unique_ptr<MergeData> &&result);
        //End Implementation for Paralleizer Traits

        //Start Implementation Interfaces For Reducer Traits
        using ReduceData = vector<CommunityPtr>;

        unique_ptr<ReduceData> WrapMergeDataToReduceData(unique_ptr<MergeData> merge_data_ptr);

        function<bool(unique_ptr<ReduceData> &, unique_ptr<ReduceData> &)> CmpReduceData;

        function<unique_ptr<ReduceData>(unique_ptr<ReduceData>,
                                        unique_ptr<ReduceData> right_data_ptr)> ReduceComputation;
        //End of Implementation For Reducer Traits

        void ExecuteDaemon();

        Daemon(double epsilon, int min_community_size, unique_ptr<Graph> graph_ptr, int max_iteration) :
                epsilon_(epsilon), min_community_size_(min_community_size), max_iteration_num_(max_iteration) {
            ;
            graph_ptr_ = std::move(graph_ptr);
            overlap_community_vec_ = make_unique<vector<CommunityPtr>>();

            CmpReduceData = [](unique_ptr<ReduceData> &left, unique_ptr<ReduceData> &right) -> bool {
                auto cmp = [](auto &&tmp_left, auto &&tmp_right) -> bool {
                    return tmp_left->size() < tmp_right->size();
                };
                auto iter1 = max_element(left->begin(), left->end(), cmp);
                auto iter2 = max_element(left->begin(), left->end(), cmp);
                return (*iter1)->size() > (*iter2)->size();
            };

            ReduceComputation = [this](
                    unique_ptr<ReduceData> left_data_ptr,
                    unique_ptr<ReduceData> right_data_ptr) -> unique_ptr<ReduceData> {
                if (left_data_ptr->size() == 0) {
                    for (auto iter_inner = right_data_ptr->begin();
                         iter_inner != right_data_ptr->end(); ++iter_inner) {
                        if ((*iter_inner)->size() > min_community_size_)
                            left_data_ptr->push_back(std::move(*iter_inner));
                    }
                }
                else {
                    for (auto iter_inner = right_data_ptr->begin(); iter_inner != right_data_ptr->end(); ++iter_inner) {
                        CommunityPtr tmp_copy_ptr;
                        bool first_access_flag = false;
                        for (auto iter = left_data_ptr->begin(); iter != left_data_ptr->end(); ++iter) {
                            auto cover_rate_result = GetTwoCommunitiesCoverRate(*iter,
                                                                                *iter_inner);

                            if (cover_rate_result > epsilon_) {
                                 MergeTwoCommunities(*iter, *iter_inner);
                                break;
                            }
                            else if ((*iter_inner)->size() > min_community_size_ && !first_access_flag) {
                                tmp_copy_ptr = make_unique<set<unsigned long>>();
                                for (auto tmp_iter = (*iter_inner)->begin();
                                     tmp_iter != (*iter_inner)->end(); ++tmp_iter) {
                                    tmp_copy_ptr->insert(*tmp_iter);
                                }
                                first_access_flag = true;
                            }
                        }
                        if (first_access_flag) {
                            left_data_ptr->push_back(std::move(tmp_copy_ptr));
                        }
                    }

                }
                return left_data_ptr;
            };
        }

    private:
        unique_ptr<Graph> graph_ptr_;

        double epsilon_;
        int min_community_size_;
        int max_iteration_num_;

        unique_ptr<SubGraph> ExtractEgoMinusEgo(Vertex ego_vertex);

        CommunityVecPtr LabelPropagationOnSubGraph(
                unique_ptr<SubGraph> sub_graph_ptr, Vertex ego_vertex);

        double GetTwoCommunitiesCoverRate(CommunityPtr &left_community, CommunityPtr &right_community);

        void MergeTwoCommunities(CommunityPtr& left_community, CommunityPtr& right_community);


    };
}

#endif //CODES_YCHE_DAMON_ALGORITHM_H



























