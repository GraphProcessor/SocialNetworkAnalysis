//
// Created by cheyulin on 2/24/16.
//

#ifndef CODES_YCHE_DAMON_ALGORITHM_H
#define CODES_YCHE_DAMON_ALGORITHM_H

#endif //CODES_YCHE_DAMON_ALGORITHM_H

#include <memory>
#include <boost/graph/adjacency_list.hpp>

using namespace boost;
using namespace std;

namespace yche {
    class Daemon {
        using Graph = adjacency_list<vecS, vecS, undirectedS>;
        using Vertex = graph_traits<Graph>::vertex_descriptor;
        using VertexIndex = int;

        //Property Tag
        struct vertex_id_t;
        struct vertex_weight_t;
        struct vertex_label_t;
        using SubGraphVertexProperty = property<vertex_id_t, VertexIndex,
                property<vertex_weight_t, int,
                        property<vertex_label_t, VertexIndex>>>;
        using SubGraph = adjacency_list<vecS, vecS, undirectedS, SubGraphVertexProperty>;
        using SubGraphVertex = graph_traits<SubGraph>::vertex_descriptor;

        using Community = unique_ptr<set<VertexIndex>>;
        using OverlappingCommunityVec = vector<Community>;

    private:
        Graph graph_;
        OverlappingCommunityVec overlapping_community_vec_;
        double epsilon_;
        int min_community_size_;

        unique_ptr<SubGraph> ExtractEgoMinusEgo(Vertex ego_vertex);

        OverlappingCommunityVec DetectCommunitesViaLabelPropogation(
                unique_ptr<Graph> sub_graph, SubGraphVertex ego_vertex);

        void MergeTwoCommunities(Community left_community, Community right_community);

        void MergeIntoOverallCommunities(OverlappingCommunityVec &new_created_communities_vec);

    public:
        Daemon(double epsilon, int min_community_size, vector<pair<int, int>> &edge_list,
               long vertices_size) :
                epsilon_(epsilon), min_community_size_(min_community_size) {
            graph_ = Graph(edge_list.begin(), edge_list.end(), vertices_size);
        }

        void ExecuteDaemon();
    };
}
