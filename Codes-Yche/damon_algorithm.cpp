//
// Created by cheyulin on 2/24/16.
//

#include "damon_algorithm.h"

namespace yche {
    using Daemon;

    unique_ptr<Daemon::SubGraph> Daemon::ExtractEgoMinusEgo(Daemon::Vertex ego_vertex) {
        unique_ptr<SubGraph> ego_net_ptr = make_unique<SubGraph>();
        auto adjacency_iterator_pair = adjacent_vertices(ego_vertex, graph_);
        auto begin = adjacency_iterator_pair.first;
        auto end = adjacency_iterator_pair.second;
        for (; begin != end; ++begin) {
//                ego_net_ptr->added_vertex()
        }
//        return std::unique_ptr<yche::Daemon::SubGraph>();
    }
}