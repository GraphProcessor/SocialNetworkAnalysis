//
// Created by cheyulin on 5/16/16.
//

#ifndef OCD_EVALUATION_YCHE_MODULARITY_H
#define OCD_EVALUATION_YCHE_MODULARITY_H

#include "include_header.h"
#include "input_output_handler.h"

using namespace boost;
using namespace std;

namespace yche {
    static constexpr double PRECISION = 0.0001;

    template<typename CoefficientFunc>
    class ModularityLinkBelonging {
    private:
        using Graph = adjacency_list<setS, vecS, undirectedS>;
        using Vertex = graph_traits<Graph>::vertex_descriptor;
        using VertexIndexType = unsigned long;

        char *input_file_str_;
        char *output_file_str_;

        unique_ptr<Graph> graph_ptr_;
        CoefficientFunc overlap_coefficient_calc_func_;

        map<VertexIndexType, Vertex> name_vertex_map_;
        vector<pair<VertexIndexType, VertexIndexType>> edge_vec_;
        vector<unique_ptr<vector<Vertex>>> overlap_communities_;

        vector<int> community_belongings_count_vec_;
        vector<double> community_belongings_count_reverse_vec_;
        vector<VertexIndexType> in_degree_vec_;
        vector<VertexIndexType> out_degree_vec_;

        void ConstructGraph();

        void ConstructCommunities();

        void InitAlphaVertexCommunity();

    public:

        ModularityLinkBelonging(char *input_file_str_, char *ouput_file_str_,
                                CoefficientFunc overlap_coefficient_calc_func_) :
                input_file_str_(input_file_str_), output_file_str_(ouput_file_str_),
                overlap_coefficient_calc_func_(overlap_coefficient_calc_func_) {
            ConstructGraph();
            ConstructCommunities();
            InitAlphaVertexCommunity();
        }

        double CalculateModularity();
    };

    template<typename CoefficientFunc>
    void ModularityLinkBelonging<CoefficientFunc>::ConstructGraph() {
        name_vertex_map_.clear();
        graph_ptr_ = make_unique<Graph>();
        yche::ReadEdgeListInToEdgeVector<VertexIndexType>(input_file_str_, edge_vec_);

        for (auto iter = edge_vec_.begin(); iter != edge_vec_.end(); ++iter) {
            if (name_vertex_map_.find(iter->first) == name_vertex_map_.end()) {
                Vertex vertex = add_vertex(*graph_ptr_);
                name_vertex_map_.insert(make_pair(iter->first, vertex));
            }
            if (name_vertex_map_.find(iter->second) == name_vertex_map_.end()) {
                Vertex vertex = add_vertex(*graph_ptr_);
                name_vertex_map_.insert(make_pair(iter->second, vertex));
            }
            add_edge(name_vertex_map_[iter->first], name_vertex_map_[iter->second], *graph_ptr_);
        }
        community_belongings_count_vec_.resize(num_vertices(*graph_ptr_), 0);
        community_belongings_count_reverse_vec_.resize(community_belongings_count_vec_.size(), 0);
    }

    template<typename CoefficientFunc>
    void ModularityLinkBelonging<CoefficientFunc>::ConstructCommunities() {
        overlap_communities_.clear();
        yche::GetOverlappingCommunities<VertexIndexType, Vertex>(output_file_str_, overlap_communities_,
                                                                 name_vertex_map_);
    }

    template<typename CoefficientFunc>
    double ModularityLinkBelonging<CoefficientFunc>::CalculateModularity() {
        double modularity_rate = 0;
        auto edge_num = num_edges(*graph_ptr_);
        auto vertices_num = num_vertices(*graph_ptr_);
        cout << edge_num << "," << vertices_num << endl;
        property_map<Graph, vertex_index_t>::type vertex_index_map = get(vertex_index, *graph_ptr_);
        cout << overlap_communities_.size() << endl;

        for (auto &community_ptr :overlap_communities_) {
            auto &community_members = *community_ptr;

            auto comm_size = community_ptr->size();
            double **f_value_matrix = new double *[comm_size];
            double *f_sum_in_arr = new double[comm_size];
            double *f_sum_out_arr = new double[comm_size];
            double *in_degree_arr = new double[comm_size];
            double *out_degree_arr = new double[comm_size];

            for(auto i=0;i<comm_size;i++){
                f_sum_in_arr[i]=0;
                f_sum_out_arr[i]=0;
                in_degree_arr[i]=0;
                out_degree_arr[i]=0;
            }

            for (auto i = 0; i < comm_size; i++) {
                f_value_matrix[i] = new double[comm_size];
                for(auto j=0;j<comm_size;j++)
                    f_value_matrix[i][j]=0;
            }

            bool adjacent_flag;
            //Calculate F_value_matrix F_sum_in F_sum_out
            for (auto i = 0; i < comm_size; i++) {
                auto start_index = vertex_index_map[community_members[i]];
                in_degree_arr[i] = in_degree_vec_[start_index];
                out_degree_arr[i] = out_degree_vec_[start_index];
                for (auto j = 0; j < comm_size; j++) {

                    auto end_index = vertex_index_map[community_members[j]];
                    if (i == j)
                        continue;
                    else {
                        adjacent_flag = edge(community_members[i], community_members[j], *graph_ptr_).second;
                        if (adjacent_flag) {
                            f_value_matrix[i][j] = overlap_coefficient_calc_func_(
                                    community_belongings_count_reverse_vec_[start_index],
                                    community_belongings_count_reverse_vec_[end_index]);
                            if(f_value_matrix[i][j]>1){
                                cout <<"shit:"<<f_value_matrix[i][j]<<endl;
                            }
                            f_sum_out_arr[i] += f_value_matrix[i][j];
                            f_sum_in_arr[j] += f_value_matrix[i][j];
                            if(f_value_matrix[i][j]>1){
                                cout <<"shit:!!!"<<f_value_matrix[i][j]<<endl;
                            }
                        }
                    }
                }
            }

            for (auto i = 0; i < comm_size; i++) {
                f_sum_in_arr[i] /= vertices_num;
                f_sum_out_arr[i] /= vertices_num;
            }

            for (auto i = 0; i < comm_size; i++) {
                for (auto j = 0; j < comm_size; j++) {
                    if (i == j)
                        continue;
                    if (f_value_matrix[i][j] > PRECISION) {
                        if(f_value_matrix[i][j]>1){
                            cout <<"shit:" << i<<","<<j<<" :"<<f_value_matrix[i][j]<<endl;
                        }
                        modularity_rate += f_value_matrix[i][j] -
                                           out_degree_arr[i] * in_degree_arr[j] * f_sum_out_arr[i] * f_sum_in_arr[j] /
                                           edge_num;
                    }
                }
            }

            delete[] in_degree_arr;
            delete[] out_degree_arr;

            delete[] f_sum_in_arr;
            delete[] f_sum_out_arr;
            for (auto i = 0; i < comm_size; i++) {
                delete[] f_value_matrix[i];
            }
            delete[] f_value_matrix;
            cout << "Curr Modularity:"<<modularity_rate/edge_num << endl <<endl;
        }

        modularity_rate = modularity_rate / edge_num;
        return modularity_rate;
    }

    template<typename CoefficientFunc>
    void ModularityLinkBelonging<CoefficientFunc>::InitAlphaVertexCommunity() {
        property_map<Graph, vertex_index_t>::type vertex_index_map = get(vertex_index, *graph_ptr_);
        for (auto &community_ptr :overlap_communities_) {
            for (auto &vertex :*community_ptr) {
                community_belongings_count_vec_[vertex_index_map[vertex]]++;
            }
        }

        for (auto i = 0; i < community_belongings_count_vec_.size(); i++) {
            if (community_belongings_count_vec_[i] != 0) {
                community_belongings_count_reverse_vec_[i] = 1.0 / community_belongings_count_vec_[i];
            }
        }
        auto num_vertices = boost::num_vertices(*graph_ptr_);
        in_degree_vec_.resize(num_vertices);
        out_degree_vec_.resize(num_vertices);
        auto graph = *graph_ptr_;
        for (auto iter = vertices(graph); iter.first != iter.second; iter.first++) {
            auto vertex = *iter.first;
            in_degree_vec_[vertex_index_map[vertex]] = in_degree(vertex, graph);
            out_degree_vec_[vertex_index_map[vertex]] = out_degree(vertex, graph);
        }
    }
}

#endif //OCD_EVALUATION_YCHE_MODULARITY_H
