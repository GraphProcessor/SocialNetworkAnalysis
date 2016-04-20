//
// Created by cheyulin on 4/15/16.
//

#ifndef CODES_YCHE_CIS_ALGORITHM_H
#define CODES_YCHE_CIS_ALGORITHM_H

#include "include_header.h"

#define DOUBLE_ACCURACY 0.00001

using namespace std;
using namespace boost;

enum vertex_id_t {
    vertex_id
};
namespace boost {
    BOOST_INSTALL_PROPERTY(vertex, id);
}

namespace yche {
    using IndexType = unsigned long;
    using CommunityMembers = set<IndexType>;

    struct CommunityInfo {
        CommunityInfo(double w_in, double w_out) : w_in_(w_in), w_out_(w_out) { }

        unique_ptr<CommunityMembers> members_;
        double w_in_;
        double w_out_;
    };

    struct MemberInfo {
        MemberInfo(IndexType member_index) : member_index_(member_index), w_in_(0), w_out_(0) { }

        MemberInfo(const MemberInfo &member_info) {
            this->member_index_ = member_info.member_index_;
            this->w_in_ = member_info.w_in_;
            this->w_out_ = member_info.w_out_;
        }

        IndexType member_index_;
        double w_in_;
        double w_out_;
    };

    class Cis {
    public:
        using EdgeProperties = property<edge_weight_t, double>;
        using VertexProperties = property<vertex_index_t, IndexType>;
        using Graph = adjacency_list<setS, vecS, undirectedS, VertexProperties, EdgeProperties>;

        using Vertex = graph_traits<Graph>::vertex_descriptor;
        using Edge = graph_traits<Graph>::edge_descriptor;

        using CommunityVec=vector<unique_ptr<CommunityMembers>>;

        Cis(unique_ptr<Graph> graph_ptr, double lambda, map<int, int> &vertex_name_map) :
                lambda_(lambda), vertex_name_map_(vertex_name_map) {
            graph_ptr_ = std::move(graph_ptr);
            vertices_.clear();
            //Init Vertices
            property_map<Graph, vertex_index_t>::type vertex_index_map = boost::get(vertex_index, *graph_ptr_);
            for (auto vp = vertices(*graph_ptr_); vp.first != vp.second; ++vp.first) {
                Vertex vertex = *vp.first;
                vertices_.push_back(*vp.first);
            }
        }


        unique_ptr<CommunityVec> ExecuteCis();

    private:
        map<int, int> vertex_name_map_;
        unique_ptr<Graph> graph_ptr_;
        vector<Vertex> vertices_;

        double lambda_;

        double CalculateDensity(const IndexType &size, const double &w_in, const double &w_out, const double &lambda);

        unique_ptr<CommunityInfo> SplitAndChooseBestConnectedComponent(unique_ptr<CommunityMembers> community_ptr);

        unique_ptr<CommunityMembers> ExpandSeed(unique_ptr<CommunityMembers> seed_member_ptr);


        double GetTwoCommunitiesCoverRate(unique_ptr<CommunityMembers> &&left_community,
                                          unique_ptr<CommunityMembers> &&right_community);

        unique_ptr<CommunityMembers> MergeTwoCommunities(unique_ptr<CommunityMembers> &&left_community,
                                                         unique_ptr<CommunityMembers> &&right_community);


    };
}


#endif //CODES_YCHE_CIS_ALGORITHM_H
