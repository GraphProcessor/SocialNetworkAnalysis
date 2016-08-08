//
// Created by cheyulin on 8/8/16.
//
#include <thread>
#include "../thread_pool_base.h"

int main() {
    using namespace yche;
    ThreadPoolBase<> pool(10); // Defaults to 10 threads.
    int JOB_COUNT = 100;

    for (int i = 0; i < JOB_COUNT; ++i)
        pool.AddTask([i]() {
            std::cout << "Hello" << i << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        });
}