//
// Created by cheyulin on 2/24/16.
//

#ifndef CODES_YCHE_DAMON_ALGORITHM_H
#define CODES_YCHE_DAMON_ALGORITHM_H

#endif //CODES_YCHE_DAMON_ALGORITHM_H

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
        using VertexProperties = property<vertex_weight_t, double,
                property<vertex_index_t, int>>;
        using Graph = adjacency_list<vecS, setS, undirectedS, VertexProperties>;

        using SubGraphVertexProperties = property<vertex_weight_t, double,
                property<vertex_id_t, int,
                        property<vertex_label_t, array<int, 2>>
                >>;

        using SubGraph = adjacency_list<vecS, vecS, undirectedS, SubGraphVertexProperties>;

        using Vertex = graph_traits<Graph>::vertex_descriptor;
        using SubGraphVertex = graph_traits<SubGraph>::vertex_descriptor;
        using Community = unique_ptr<set<int>>;
        using OverlappingCommunityVec = vector<Community>;

    private:
        unique_ptr<Graph> graph_ptr_;
        OverlappingCommunityVec overlapping_community_vec_;
        double epsilon_;
        int min_community_size_;
        int max_iteration_num_;

        unique_ptr<SubGraph> ExtractEgoMinusEgo(Vertex ego_vertex);

        OverlappingCommunityVec DetectCommunitesViaLabelPropagation(
                unique_ptr<Graph> sub_graph, SubGraphVertex ego_vertex);

        void MergeTwoCommunities(Community left_community, Community right_community);

        void MergeIntoOverallCommunities(OverlappingCommunityVec &new_created_communities_vec);

    public:
        Daemon(double epsilon, int min_community_size, unique_ptr<Graph> graph_ptr, int max_iteration) :
                epsilon_(epsilon), min_community_size_(min_community_size), max_iteration_num_(max_iteration) {
            ;
            std::move(graph_ptr);
            graph_ptr_ = std::move(graph_ptr);
        }

        void ExecuteDaemon();
    };
}
