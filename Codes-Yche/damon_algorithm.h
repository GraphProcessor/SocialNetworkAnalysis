//
// Created by cheyulin on 2/24/16.
//

#ifndef CODES_YCHE_DAMON_ALGORITHM_H
#define CODES_YCHE_DAMON_ALGORITHM_H

#endif //CODES_YCHE_DAMON_ALGORITHM_H

#include "include_header.h"

using namespace boost;
using namespace std;

namespace yche {
    using VertexIndexType = int;
    //Property Tag
    struct  SubGraphVertexProperties{
        typedef vertex_property_type type;
    };
    struct vertex_id_t{
        typedef vertex_property_type type;
    };
    struct vertex_weight_t{
        typedef vertex_property_type type;
    };
    struct vertex_label_t{
        typedef vertex_property_type type;
    };

    class Daemon {
        using Graph = adjacency_list<vecS, setS, undirectedS,property<vertex_weight_t,double >>;
        using VertexDescriptor = graph_traits<Graph>::vertex_descriptor;

        using GraphVertexProperty= property<vertex_weight_t, double>;

        using SubGraphVertexProperty = property<vertex_id_t, VertexIndexType,
                property<vertex_weight_t, double,
                        property<vertex_label_t, array<VertexIndexType, 2>>>>;
        using SubGraph = adjacency_list<vecS, vecS, undirectedS, SubGraphVertexProperty>;
        using SubGraphVertexDescriptor = graph_traits<SubGraph>::vertex_descriptor;

        using Community = unique_ptr<set<VertexIndexType>>;
        using OverlappingCommunityVec = vector<Community>;

    private:
        Graph graph_;
        OverlappingCommunityVec overlapping_community_vec_;
        double epsilon_;
        int min_community_size_;
        int max_iteration_num_;

        unique_ptr<SubGraph> ExtractEgoMinusEgo(VertexDescriptor ego_vertex);

        OverlappingCommunityVec DetectCommunitesViaLabelPropagation(
                unique_ptr<Graph> sub_graph, SubGraphVertexDescriptor ego_vertex);

        void MergeTwoCommunities(Community left_community, Community right_community);

        void MergeIntoOverallCommunities(OverlappingCommunityVec &new_created_communities_vec);

    public:
        Daemon(double epsilon, int min_community_size, vector<pair<int, int>> &edge_list,
               long vertices_size, int max_iteration) :
                epsilon_(epsilon), min_community_size_(min_community_size), max_iteration_num_(max_iteration) {
            graph_ = Graph(edge_list.begin(), edge_list.end(), vertices_size);

        }

        void ExecuteDaemon();
    };
}
