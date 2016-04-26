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

        Daemon(double epsilon, int min_community_size, unique_ptr<Graph> graph_ptr, int max_iteration) :
                epsilon_(epsilon), min_community_size_(min_community_size), max_iteration_num_(max_iteration) {
            ;
            graph_ptr_ = std::move(graph_ptr);
            overlap_community_vec_ = make_unique<vector<CommunityPtr>>();
        }

        //Implemt Interfaces For Parallelizer
        using BasicData = Vertex;
        using MergeData = vector<CommunityPtr>;

        unique_ptr<vector<unique_ptr<BasicData>>> InitBasicComputationData();

        unique_ptr<MergeData> LocalComputation(unique_ptr<BasicData> seed_member_ptr);

        void MergeToGlobal(unique_ptr<MergeData> &&result);
        //End Implentation for Paralleizer Traits

        void ExecuteDaemon();

        CommunityVecPtr overlap_community_vec_;

    private:
        unique_ptr<Graph> graph_ptr_;

        double epsilon_;
        int min_community_size_;
        int max_iteration_num_;

        unique_ptr<SubGraph> ExtractEgoMinusEgo(Vertex ego_vertex);

        CommunityVecPtr LabelPropagationOnSubGraph(
                unique_ptr<SubGraph> sub_graph_ptr, Vertex ego_vertex);

        pair<double, pair<CommunityPtr, CommunityPtr>> GetTwoCommunitiesCoverRate(CommunityPtr left_community,
                                                                                  CommunityPtr right_community);

        pair<CommunityPtr, CommunityPtr> MergeTwoCommunities(CommunityPtr left_community, CommunityPtr right_community);


    };
}

#endif //CODES_YCHE_DAMON_ALGORITHM_H



























