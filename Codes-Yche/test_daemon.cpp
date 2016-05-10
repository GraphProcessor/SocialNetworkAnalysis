#include "damon_algorithm.h"
#include "parallelizer.h"

int main(int argc, char *argv[]) {

    using namespace yche;
    long thread_num = atol(argv[1]);

    char *file_name_ptr = argv[2];
//    string file = "/home/cheyulin/gitrepos/SocialNetworkAnalysis/Codes-Yche/karate_edges_input.csv";
//    string file = "/home/cheyulin/gitrepos/SocialNetworkAnalysis/Codes-Yche/collaboration_edges_input.csv";
//    string file = "/home/cheyulin/gitrepos/SocialNetworkAnalysis/Dataset/social_network/twitter_combined.txt";
//    ifstream fin(file.c_str());
    ifstream fin(file_name_ptr);
    string s;
    if (!fin) {
        cout << "Error opening " << string(file_name_ptr) << " for input" << endl;
        exit(-1);
    }

    int i = 0;
    vector<pair<int, int>> edges_vec;
    while (getline(fin, s)) {
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


    unique_ptr<Daemon::Graph> graph_ptr = make_unique<Daemon::Graph>();
    map<int, Daemon::Vertex> name_vertex_map;

    using namespace boost;
    property_map<Daemon::Graph, vertex_weight_t>::type vertex_weight_map =
            get(vertex_weight, *graph_ptr);
    for (auto iter = edges_vec.begin(); iter != edges_vec.end(); ++iter) {
        if (name_vertex_map.find(iter->first) == name_vertex_map.end()) {
            Daemon::Vertex vertex = add_vertex(*graph_ptr);
            vertex_weight_map[vertex] = 1;
            name_vertex_map.insert(make_pair(iter->first, vertex));
        }
        if (name_vertex_map.find(iter->second) == name_vertex_map.end()) {
            Daemon::Vertex vertex = add_vertex(*graph_ptr);
            vertex_weight_map[vertex] = 1;
            name_vertex_map.insert(make_pair(iter->second, vertex));
        }
        add_edge(name_vertex_map[iter->first], name_vertex_map[iter->second], *graph_ptr);
    }

    map<int, int> index_name_map;
    property_map<Daemon::Graph, vertex_index_t>::type vertex_index_map = get(vertex_index, *graph_ptr);
    for (auto iter = name_vertex_map.begin(); iter != name_vertex_map.end(); ++iter) {
        index_name_map.insert(make_pair(vertex_index_map[iter->second], iter->first));
//        cout << iter->first << "," << vertex_index_map[iter->second] << endl;
    }

    auto epsilon = 0.5;
    auto min_community_size = 3;
    auto max_iteration = 100;
    unique_ptr<Daemon> daemon_ptr = make_unique<Daemon>(epsilon, min_community_size, std::move(graph_ptr),
                                                        max_iteration);
    Parallelizer<Daemon> parallelizer(thread_num, std::move(daemon_ptr));


    parallelizer.ParallelExecute();
//    daemon_ptr.ExecuteDaemon();
    daemon_ptr = std::move(parallelizer.algorithm_ptr_);
    if (daemon_ptr->overlap_community_vec_ == nullptr)
        cout << "shit" << endl;
    else
        cout << daemon_ptr->overlap_community_vec_->size() << endl;
    auto communities = std::move(daemon_ptr->overlap_community_vec_);
    int count = 0;

    cout << "comm_size:" << communities->size() << endl;
    for (auto iter = communities->begin(); iter != communities->end(); ++iter) {
        count++;
        cout << "Comm" << count << endl;
        for (auto iter_inner = (*iter)->begin(); iter_inner != (*iter)->end(); iter_inner++) {
            cout << index_name_map[*iter_inner] << " ";
        }
        cout << endl;
    }

//    getchar();

    return 0;
}

