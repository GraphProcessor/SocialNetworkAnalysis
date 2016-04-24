//
// Created by cheyulin on 4/20/16.
//

#ifndef CODES_YCHE_GCE_ALGORITHM_H
#define CODES_YCHE_GCE_ALGORITHM_H

#include "include_header.h"

namespace yche {
    using namespace std;
    using namespace boost;
    using IndexType  = unsigned long;

    struct MemberInfo {
        MemberInfo(IndexType member_index_, double w_out_ = 0, double w_in_ = 0) :
                member_index_(member_index_), w_out_(w_out_), w_in_(w_in_) { }

        IndexType member_index_;
        double w_out_;
        double w_in_;
    };

    class CommunityInfo {
    public:
        static map<IndexType, unique_ptr<CommunityInfo>> node_community_map_;

    private:
        set<IndexType> members_;

        void GetOverlapRate();

        bool IsOverlapWithAlredaySeeds();

        void InitWinWoutAndNeighbors();

        IndexType GetBestFitNeighbor();

        void AddNeighborToMember(IndexType member_index);

        double w_out_;
        double w_in_;
    };

    class Gce {
    public:
        using EdgeProperties = property<edge_weight_t, double>;
        using VertexProperties = property<vertex_index_t, IndexType>;
        using Graph = adjacency_list<setS, vecS, undirectedS, VertexProperties, EdgeProperties>;

        using Vertex = graph_traits<Graph>::vertex_descriptor;
        using Edge = graph_traits<Graph>::edge_descriptor;

        static double CalFitnessFunction(const double &w_in, const double &w_out);

    private:
        double alpha_parameter_;
        double overlap_seed_rate_threshold_;
        int overlap_seed_num_threshold_;
        
    };
}


#endif //CODES_YCHE_GCE_ALGORITHM_H
