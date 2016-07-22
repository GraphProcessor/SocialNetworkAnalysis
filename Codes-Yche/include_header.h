//
// Created by cheyulin on 2/24/16.
//

#ifndef CODES_YCHE_INCLUDE_HEADER_H
#define CODES_YCHE_INCLUDE_HEADER_H

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <map>
#include <memory>
#include <ctime>
#include <iostream>
#include <fstream>
#include <sstream>
#include <random>
#include <limits>
#include <queue>
#include <boost/graph/adjacency_list.hpp>
#include <boost/regex.hpp>
#include "semaphore.h"
#include <pthread.h>

namespace yche {
    //Define Tags For Template Specialization In parallelizer.h
    struct MergeWithReduce;
    struct MergeSequential;
}

#endif //CODES_YCHE_INCLUDE_HEADER_H

