//
// Created by cheyulin on 5/16/16.
//

#ifndef OCD_EVALUATION_YCHE_MODULARITY_H
#define OCD_EVALUATION_YCHE_MODULARITY_H

#include "include_header.h"

using namespace boost;

template<typename CoefficientFunc, typename >
class Modularity {
    CoefficientFunc overlap_coefficient_calc_func;
    using Graph = adjacency_list<setS, vecS, undirectedS>;
    using Vertex = graph_traits<Graph>::vertex_descriptor;

};


#endif //OCD_EVALUATION_YCHE_MODULARITY_H
