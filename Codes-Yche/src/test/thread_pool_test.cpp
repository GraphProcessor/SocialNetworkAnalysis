//
// Created by cheyulin on 8/8/16.
//
#include <thread>
#include "../parallel_utils/thread_pool_base.h"
#include <iostream>

using std::cout;
using std::endl;

int main() {
    using namespace yche;
    ThreadPoolBase<int> pool(20);
    int JOB_COUNT = 5000;

    for (auto j = 0; j < 300; j++) {
        cout << "Round:" << j << endl;
        for (int i = 0; i < JOB_COUNT; ++i) {
            std::function<int(void)> task_function = [i]() {
                return i * i;
            };
            pool.AddTask(task_function);
        }
        cout << "Finish Add" << endl;
        pool.WaitAll();
    }
}