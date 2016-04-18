//
// Created by cheyulin on 4/18/16.
//

#include <iostream>

using namespace std;

#include "cis_algorithm.h"

using namespace std;

int main() {
    using namespace yche;

    string file = "/home/cheyulin/gitrepos/SocialNetworkAnalysis/Codes-Yche/karate_edges_input.csv";
    ifstream fin(file.c_str());
    string s;
    if (!fin) {
        cout << "Error opening " << file << " for input" << endl;
        exit(-1);
    }

    int i = 0;
    vector<pair<int, int>> edges_vec;
    while (getline(fin, s)) {
        int first_vertex_name;
        int second_vertex_name;
        stringstream string_stream;
        string_stream.clear();
        string_stream.str(s);
        string_stream >> first_vertex_name;
        string_stream >> second_vertex_name;
        edges_vec.push_back(make_pair(first_vertex_name, second_vertex_name));
        i++;
    }


    unique_ptr<Cis::Graph> graph_ptr = make_unique<Cis::Graph>();
    map<int, Cis::Vertex> name_vertex_map;

    using namespace boost;
    property_map<Cis::Graph, edge_weight_t>::type edge_weight_map =
            get(edge_weight, *graph_ptr);
    for (auto iter = edges_vec.begin(); iter != edges_vec.end(); ++iter) {
        if (name_vertex_map.find(iter->first) == name_vertex_map.end()) {
            Cis::Vertex vertex = add_vertex(*graph_ptr);
            name_vertex_map.insert(make_pair(iter->first, vertex));
        }
        if (name_vertex_map.find(iter->second) == name_vertex_map.end()) {
            Cis::Vertex vertex = add_vertex(*graph_ptr);
            name_vertex_map.insert(make_pair(iter->second, vertex));
        }
        Cis::Edge edge;
        bool flag;
        tie(edge, flag) = add_edge(name_vertex_map[iter->first], name_vertex_map[iter->second], *graph_ptr);
        edge_weight_map[edge] = 1;
    }

    map<int, int> index_name_map;
    property_map<Cis::Graph, vertex_index_t>::type vertex_index_map = get(vertex_index, *graph_ptr);
    for (auto iter = name_vertex_map.begin(); iter != name_vertex_map.end(); ++iter) {
        index_name_map.insert(make_pair(vertex_index_map[iter->second], iter->first));
        cout << iter->first << "," << vertex_index_map[iter->second] << endl;
    }

    cout << "hello" << endl << endl;
    Cis cis(std::move(graph_ptr), 0);
    auto communities_ptr_vec = std::move(cis.ExecuteCis());
    for (auto &&community_ptr:*communities_ptr_vec) {
        for (auto member_id:*community_ptr) {
            cout << index_name_map[member_id] << ",";
        }
        cout << endl;
    }
    getchar();
    return 0;
}

