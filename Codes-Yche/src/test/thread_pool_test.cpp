//
// Created by cheyulin on 8/8/16.
//
#include <thread>
#include "../parallel_utils/thread_pool_base.h"

int main() {
    using namespace yche;
    ThreadPoolBase<int> pool(20);
    int JOB_COUNT = 500;

    for (int i = 0; i < JOB_COUNT; ++i)
        pool.AddTask([i]() -> int {
            std::cout << "Hello" << i << std::endl;
            return (i * i);
        });
}