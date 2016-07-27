//
// Created by cheyulin on 5/12/16.
//

#ifndef CODES_YCHE_INPUT_OUTPUT_HANDLER_H
#define CODES_YCHE_INPUT_OUTPUT_HANDLER_H

#include <boost/regex.hpp>
#include <fstream>
#include <sstream>
#include <memory>
#include <map>
#include <vector>

namespace yche {
    using namespace std;

    template<typename VertexIndexType>
    void ReadEdgeListInToEdgeVector(char *const &file_name_ptr,
                                    vector<pair<VertexIndexType, VertexIndexType>> &edges_vec) {
        ifstream fin(file_name_ptr);
        string s;
        if (!fin) {
            cout << "Error opening " << string(file_name_ptr) << " for input" << endl;
            exit(-1);
        }

        int i = 0;
        while (getline(fin, s)) {
            using namespace boost;
            boost::regex pat("$#.*");
            boost::smatch matches;
            if (boost::regex_match(s, matches, pat))
                continue;

            int first_vertex_name = -1;
            int second_vertex_name = -1;
            stringstream string_stream;
            string_stream.clear();
            string_stream.str(s);
            string_stream >> first_vertex_name;
            string_stream >> second_vertex_name;
            edges_vec.push_back(make_pair(first_vertex_name, second_vertex_name));
            i++;
        }
    }

    template<typename Algorithm, typename VertexIndexType>
    void ExecuteAlgorithmWithParallelizer(const unsigned long &thread_num,
                                          unique_ptr<Algorithm> &algorithm_ptr,
                                          map<VertexIndexType, VertexIndexType> &index_name_map) {

        cout << "Reduce Enabled" << endl;
        Parallelizer<Algorithm> parallelizer(thread_num, std::move(algorithm_ptr));
        parallelizer.ParallelExecute();
        algorithm_ptr = std::move(parallelizer.algorithm_ptr_);

        //Print the result
#ifndef NOT_COUT_COMMUNITY_RESULT
        auto communities_ptr_vec = std::move(algorithm_ptr->overlap_community_vec_);
        cout << "comm_size:" << communities_ptr_vec->size() << endl;
        for (auto &&community_ptr:*communities_ptr_vec) {
            for (auto member_id:*community_ptr) {
                cout << index_name_map[member_id] << "(" << member_id << "),";
            }
            cout << endl;
        }
#endif

    }
}
#endif //CODES_YCHE_INPUT_OUTPUT_HANDLER_H
