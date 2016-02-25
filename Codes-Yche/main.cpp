#include <iostream>

using namespace std;

#include "damon_algorithm.h"

int main() {
    srand(time(nullptr));
    for(auto i=0;i<10;i++){
        cout << rand()*10;
    }
    cout << "Hello, World!" << endl;
    return 0;
}