//
// Created by cheyulin on 4/15/16.
//

#include "cis_algorithm.h"

namespace yche {
    double Cis::CalDensity(const int &size, const double &w_in, const double &w_out, const double &lambda) {
        if (size < 1) return numeric_limits<double>::min();
        double partA = ((1 - lambda) * (w_in / (w_in + w_out)));
        double partB = (lambda * ((2 * w_in) / (size * (size - 1))));
        if (size == 1) partB = lambda;
        return partA + partB;
    }

    void Cis::ExpandSeed() {

    }

    void Cis::SplitIntoConnectedComponents(unique_ptr<CommunityMember> community) {
        queue<IndexType> frontier;
        while (community->size() > 0) {
            IndexType first_vertex = *community->begin();
            frontier.push(first_vertex);
            unique_ptr<CommunityInfo> community_info_ptr;
            while (frontier.size() > 0) {
                property_map<Graph, vertex_index_t>::type vertex_index_map = boost::get(vertex_index,graph_);
                for(auto vp = adjacent_vertices(graph_))
            }
        }
    }


}


