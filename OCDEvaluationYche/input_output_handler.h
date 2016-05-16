//
// Created by cheyulin on 5/16/16.
//

#ifndef OCD_EVALUATION_YCHE_INPUT_OUTPUT_HANDLER_H
#define OCD_EVALUATION_YCHE_INPUT_OUTPUT_HANDLER_H

#include "include_header.h"

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

template<typename VertexIndexType>
void GetOverlappingCommunities(char *const &output_file_name_ptr,
                               vector<unique_ptr<vector<VertexIndexType>>> &overlap_communities,
                               map<VertexIndexType, VertexIndexType> &name_index_map) {
    ifstream fin(output_file_name_ptr);
    string s;
    if (!fin) {
        cout << "Error opening " << string(output_file_name_ptr) << " for input" << endl;
        exit(-1);
    }

    boost::regex pat_start("$comm_size.*");
    boost::regex pat_num_list("[0-9]+.*");
    boost::smatch matches;
    bool is_begin_read = false;
    stringstream str_stream;
    while (getline(fin, s)) {
        using namespace boost;
        if (!is_begin_read) {
            if (boost::regex_match(s, matches, pat_start))
                is_begin_read = true;
            else
                continue;
        }
        else if (boost::regex_match(s, matches, pat_num_list)) {
            boost::regex re(",");
            s = boost::regex_replace(s, re, " ");
            str_stream.clear();
            str_stream.str(s);
            auto community_ptr = make_unique<vector<VertexIndexType>>();
            VertexIndexType tmp;
            while (true) {
                str_stream >> tmp;
                community_ptr->push_back(name_index_map[tmp]);
                if (str_stream.eof())
                    break;
            }
            overlap_communities.push_back(community_ptr);
        }
    }

}

#endif //OCD_EVALUATION_YCHE_INPUT_OUTPUT_HANDLER_H
