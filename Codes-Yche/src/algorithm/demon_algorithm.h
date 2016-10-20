//
// Created by cheyulin on 2/24/16.
//

#ifndef CODES_YCHE_DAMON_ALGORITHM_H
#define CODES_YCHE_DAMON_ALGORITHM_H

#include <boost/graph/adjacency_list.hpp>
#include <boost/range.hpp>
#include <memory>
#include <functional>
#include <vector>
#include <random>
#include <iostream>

using namespace boost;
using namespace std;

//Add User-Defined Tags For Boost Graph Library Usage
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
    class Demon {
    public:
        using IndexType = unsigned long;
        using VertexProperties = property<vertex_weight_t, double, property<vertex_index_t, IndexType >>;
        using Graph = adjacency_list<hash_setS, vecS, undirectedS, VertexProperties>;
        using Vertex = graph_traits<Graph>::vertex_descriptor;

        using SubGraphVertexProperties = property<vertex_weight_t, double,
                property<vertex_id_t, IndexType,
                        property<vertex_label_t, array<IndexType, 2>>>>;
        using SubGraph = adjacency_list<hash_setS, vecS, undirectedS, SubGraphVertexProperties>;
        using SubGraphVertex = graph_traits<SubGraph>::vertex_descriptor;

        using CommunityPtr = unique_ptr<vector<IndexType>>;
        using CommunityVecPtr = unique_ptr<vector<CommunityPtr>>;

        CommunityVecPtr overlap_community_vec_;
        using BasicDataType = Vertex;
        using MergeDataType = vector<CommunityPtr>;

        unique_ptr<vector<unique_ptr<BasicDataType>>> InitBasicComputationData();

        unique_ptr<MergeDataType> LocalComputation(unique_ptr<BasicDataType> seed_member_ptr);

        void MergeToGlobal(unique_ptr<MergeDataType> &result);

        //Start Implementation Interfaces For ReduceScheduler Traits
        using ReduceDataType = vector<CommunityPtr>;

        unique_ptr<ReduceDataType> WrapMergeDataToReduceData(unique_ptr<MergeDataType> &merge_data_ptr);

        function<bool(unique_ptr<ReduceDataType> &, unique_ptr<ReduceDataType> &)> CmpReduceData;

        function<unique_ptr<ReduceDataType>(unique_ptr<ReduceDataType> &,
                                            unique_ptr<ReduceDataType> &right_data_ptr)> ReduceComputation;


        //Start Implementation Interfaces For Fine-Grained-Merge-Scheduler Traits
        using ElementReferenceType = typename boost::range_reference<ReduceDataType>::type;

        function<bool(ElementReferenceType, ElementReferenceType)> PairMergeComputation;

        function<void(ElementReferenceType, ElementReferenceType)> SuccessAction;

        function<void(ElementReferenceType, unique_ptr<ReduceDataType> &)> FailAction;

        [[deprecated("Replaced With Parallel Execution")]]
        void ExecuteDaemon();

        Demon(double epsilon, int min_community_size, unique_ptr<Graph> graph_ptr, int max_iteration) :
                epsilon_(epsilon), min_community_size_(min_community_size), max_iteration_num_(max_iteration) {
            ;
            graph_ptr_ = std::move(graph_ptr);
            overlap_community_vec_ = make_unique<vector<CommunityPtr>>();

            CmpReduceData = [](unique_ptr<ReduceDataType> &left, unique_ptr<ReduceDataType> &right) -> bool {
                auto cmp = [](auto &tmp_left, auto &tmp_right) -> bool {
                    return tmp_left->size() < tmp_right->size();
                };
                auto iter1 = max_element(left->begin(), left->end(), cmp);
                auto iter2 = max_element(left->begin(), left->end(), cmp);
                return (*iter1)->size() > (*iter2)->size();
            };

            ReduceComputation = [this](unique_ptr<ReduceDataType> &left_data_ptr,
                                       unique_ptr<ReduceDataType> &right_data_ptr) -> unique_ptr<ReduceDataType> {
                MergeToCommunityCollection(left_data_ptr, right_data_ptr);
                return std::move(left_data_ptr);
            };

            //Start Implementation Interfaces For Fine-Grained-Merge-Scheduler Traits
            PairMergeComputation = [this](ElementReferenceType &left_element_ptr,
                                          ElementReferenceType &right_element_ptr) -> bool {
                return GetTwoCommunitiesCoverRate(left_element_ptr, right_element_ptr) > this->epsilon_;
            };

            SuccessAction = [this](ElementReferenceType left_element_ptr, ElementReferenceType right_element_ptr) {
                MergeTwoCommunitiesToLeftOne(right_element_ptr, left_element_ptr);
            };

            FailAction = [this](ElementReferenceType left_element_ptr, unique_ptr<ReduceDataType> &reduce_data_ptr) {
                if (left_element_ptr->size() > this->min_community_size_) {
                    reduce_data_ptr->push_back(std::move(left_element_ptr));
                }
            };
        }

    private:
        unique_ptr<Graph> graph_ptr_;

        double epsilon_;
        int min_community_size_;
        int max_iteration_num_;

        unique_ptr<SubGraph> ExtractEgoMinusEgo(Vertex ego_vertex);

        void DoLabelPropagationOnSingleVertex(unique_ptr<SubGraph> &sub_graph_ptr, SubGraphVertex &sub_graph_Vertex,
                                              std::mt19937 &rand_generator, const int &last_index_indicator,
                                              const int &curr_index_indicator,
                                              property_map<SubGraph, vertex_weight_t>::type sub_vertex_weight_map,
                                              property_map<SubGraph, vertex_label_t>::type sub_vertex_label_map);


        CommunityVecPtr GetCommunitiesBasedOnLabelPropagationResult(unique_ptr<SubGraph> &sub_graph_ptr,
                                                                    Vertex &ego_vertex,
                                                                    const int &curr_index_indicator);

        CommunityVecPtr DoLabelPropagationOnSubGraph(unique_ptr<SubGraph> sub_graph_ptr, Vertex ego_vertex);

        double GetTwoCommunitiesCoverRate(CommunityPtr &left_community, CommunityPtr &right_community);

        void MergeTwoCommunitiesToLeftOne(CommunityPtr &left_community, CommunityPtr &right_community);

        void MergeToCommunityCollection(decltype(overlap_community_vec_) &community_collection,
                                        unique_ptr<MergeDataType> &result);

    };
}

#endif //CODES_YCHE_DAMON_ALGORITHM_H
