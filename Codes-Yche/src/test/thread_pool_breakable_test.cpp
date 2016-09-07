//
// Created by cheyulin on 8/9/16.
//

#include <iostream>
#include "../parallel_utils/thread_pool_breakable.h"

using namespace yche;
using namespace std;

int main() {
    ThreadPoolBreakable breakable_pool(5);
    bool is_break = false;
    for (auto j = 0; j < 300; j++) {
        cout << "Round:" << j << endl;
        auto integer = 0;
        for (auto i = 0; i < 5000; i++) {
            integer++;
            std::function<BreakWithCallBackRetType(void)> task_function = [integer]() -> BreakWithCallBackRetType {
                if (integer == 10) {
                    return BreakWithCallBackRetType(true, []() { cout << "Cur Break" << endl; });
                } else
                    return BreakWithCallBackRetType();
            };
            breakable_pool.AddTask(task_function);
        }
        breakable_pool.WaitForBreakOrTerminate(is_break);
    }
}