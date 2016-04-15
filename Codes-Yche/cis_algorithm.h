//
// Created by cheyulin on 4/15/16.
//

#ifndef CODES_YCHE_CIS_ALGORITHM_H
#define CODES_YCHE_CIS_ALGORITHM_H

#include "include_header.h"

using namespace std;
using namespace boost;

enum vertex_id_t {
    vertex_id
};
namespace boost {
    BOOST_INSTALL_PROPERTY(vertex, id);
}

namespace yche {
    using IndexType = unsigned long;
    using CommunityMember = set<IndexType>;
    struct CommunityInfo {
        CommunityMember memer_;
        double w_in_;
        double w_out_;
    };

    class Cis {
    public:
        using EdgeProperties = property<edge_weight_t, double>;
        using VertexProperties = property<vertex_index_t, IndexType>;
        using Graph = adjacency_list<setS, vecS, undirectedS, VertexProperties, EdgeProperties>;

//        using SubVertexProperties = property<vertex_index_t, IndexType, property<vertex_id_t, IndexType>>;
//        using SubGraph = adjacency_list<setS, vecS, undirectedS, SubVertexProperties, EdgeProperties>;
        using Vertex = graph_traits<Graph>::vertex_descriptor;
        using Edge = graph_traits<Graph>::edge_descriptor;


        Cis(const unique_ptr<Graph> &graph_ptr, double lambda) : lambda_(lambda) {
            graph_ptr_ = std::move(graph_ptr);
        }

    private:
        unique_ptr<Graph> graph_ptr_;
        vector<Vertex> vertices_;

        double lambda_;

        double CalDensity(const int &size, const double &w_in, const double &w_out, const double &lambda);

        void SplitIntoConnectedComponents(unique_ptr<CommunityMember> community);

        void ExpandSeed();

    };
}


#endif //CODES_YCHE_CIS_ALGORITHM_H
