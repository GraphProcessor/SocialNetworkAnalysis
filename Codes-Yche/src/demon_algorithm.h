//
// Created by cheyulin on 2/24/16.
//

#ifndef CODES_YCHE_DAMON_ALGORITHM_H
#define CODES_YCHE_DAMON_ALGORITHM_H

#include <boost/graph/adjacency_list.hpp>
#include <memory>
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
        //Graph Representation Related Types
        using IndexType = unsigned long;
        using VertexProperties = property<vertex_weight_t, double,
                property<vertex_index_t, IndexType >>;
        using Graph = adjacency_list<hash_setS, vecS, undirectedS, VertexProperties>;
        using Vertex = graph_traits<Graph>::vertex_descriptor;

        //Label Propagation Related Types
        using SubGraphVertexProperties = property<vertex_weight_t, double,
                property<vertex_id_t, IndexType,
                        property<vertex_label_t, array<IndexType, 2>>>>;
        using SubGraph = adjacency_list<hash_setS, vecS, undirectedS, SubGraphVertexProperties>;
        using SubGraphVertex = graph_traits<SubGraph>::vertex_descriptor;

        //Overlapping Community Results Related Types
        using CommunityPtr = unique_ptr<vector<IndexType>>;
        using CommunityVecPtr = unique_ptr<vector<CommunityPtr>>;

        //Start Implementation Interfaces For Parallelizer Traits
        CommunityVecPtr overlap_community_vec_;
        using BasicData = Vertex;
        using MergeData = vector<CommunityPtr>;

        unique_ptr<vector<unique_ptr<BasicData>>> InitBasicComputationData();

        unique_ptr<MergeData> LocalComputation(unique_ptr<BasicData> seed_member_ptr);

        void MergeToGlobal(unique_ptr<MergeData> &result);

        //Start Implementation Interfaces For Reducer Traits
        using ReduceData = vector<CommunityPtr>;

        unique_ptr<ReduceData> WrapMergeDataToReduceData(unique_ptr<MergeData>& merge_data_ptr);

        function<bool(unique_ptr<ReduceData> &, unique_ptr<ReduceData> &)> CmpReduceData;

        function<unique_ptr<ReduceData>(unique_ptr<ReduceData>&,
                                        unique_ptr<ReduceData>& right_data_ptr)> ReduceComputation;

        [[deprecated("Replaced With Parallel Execution")]]
        void ExecuteDaemon();

        Demon(double epsilon, int min_community_size, unique_ptr<Graph> graph_ptr, int max_iteration) :
                epsilon_(epsilon), min_community_size_(min_community_size), max_iteration_num_(max_iteration) {
            ;
            graph_ptr_ = std::move(graph_ptr);
            overlap_community_vec_ = make_unique<vector<CommunityPtr>>();

            CmpReduceData = [](unique_ptr<ReduceData> &left, unique_ptr<ReduceData> &right) -> bool {
                auto cmp = [](auto &tmp_left, auto &tmp_right) -> bool {
                    return tmp_left->size() < tmp_right->size();
                };
                auto iter1 = max_element(left->begin(), left->end(), cmp);
                auto iter2 = max_element(left->begin(), left->end(), cmp);
                return (*iter1)->size() > (*iter2)->size();
            };

            ReduceComputation = [this](unique_ptr<ReduceData>& left_data_ptr,
                    unique_ptr<ReduceData>& right_data_ptr) -> unique_ptr<ReduceData> {
                MergeToCommunityCollection(left_data_ptr, right_data_ptr);
                return std::move(left_data_ptr);
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

        void MergeToCommunityCollection(decltype(overlap_community_vec_) &community_collection,unique_ptr<MergeData> &result);

    };
}

#endif //CODES_YCHE_DAMON_ALGORITHM_H
