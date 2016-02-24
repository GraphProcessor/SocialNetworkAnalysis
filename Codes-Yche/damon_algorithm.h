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

class Daemon {
    using Graph = adjacency_list<vecS, vecS, undirectedS>;
    using SubGraph = adjacency_list<vecS, vecS, undirectedS>;
    using OverlappingCommunityVec = vector<unique_ptr<set<int>>>;
    using Vertex = graph_traits<Graph>::vertex_descriptor;

private:
    unique_ptr<Graph> graph_uptr_;
    OverlappingCommunityVec overlapping_community_vec_;
    double epsilon_;
    int min_community_size_;


    unique_ptr<Graph> ExtractEgoMinusEgo();

    void DetectCommunitesViaLabelPropogation(unique_ptr<Graph> subgraph, unique_ptr<> ego_vertex);

public:
    void Execute();
};