//
// Created by cheyulin on 8/9/16.
//

#include <iostream>
#include "../thread_pool_breakable.h"

using namespace yche;
using namespace std;

int main() {
    ThreadPoolBreakable breakable_pool(10);
    auto integer = 0;
    bool is_break = false;

    for (auto i = 0; i < 500; i++) {
        integer++;
        std::function<BreakWithCallBackRetType(void)> task_function = [integer]() -> BreakWithCallBackRetType {
            if (integer == 10) {
                cout << "Go Break" << endl;
                return BreakWithCallBackRetType(true, []() { cout << "Cur Break" << endl; });
            }
            else
                return BreakWithCallBackRetType();
        };
        breakable_pool.AddTask(task_function);
    }
    breakable_pool.WaitForBreakOrTerminate(is_break);
    breakable_pool.JoinAll();
}