#include <iostream>

using namespace std;

#include "damon_algorithm.h"
using namespace std;
int main() {
    using namespace yche;
    auto epsilon =0.3;
    auto  min_community_size =3;
    auto max_iteration =20;

    string file ="/home/cheyulin/gitrepos/SocialNetworkAnalysis/Codes-Yche/karate_edges_input.csv";
    ifstream fin(file.c_str());
    string s;
    if( !fin )
    {
        cout << "Error opening " << file << " for input" << endl;
        exit(-1);
    }

    int i=0;
    while(getline(fin,s))
    {
        cout << "Read from file: " << s << endl;
        i++;
    }
    cout << "?"<<i <<endl;
    return 0;
}

