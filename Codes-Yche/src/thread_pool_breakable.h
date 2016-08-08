//
// Created by cheyulin on 8/8/16.
//

#ifndef CODES_YCHE_THREAD_POOL_BREAKABLE_H
#define CODES_YCHE_THREAD_POOL_BREAKABLE_H

#include "thread_pool_base.h"

namespace yche {

    /*
     * traits: if NextTask() return true, break
     */

    class ThreadPoolBreakable : public ThreadPoolBase<bool> {
    private:
        //introduced for break current phase computation and go into another one
        atomic_bool is_breaking_{false};

    protected:
        virtual void DoThreadFunction() override {
            while (!is_ending_) {
                //Call NextTask to get the callable function object
                if (!is_breaking_ && NextTask()()) {
                    is_breaking_ = true;
                };
                --left_tasks_counter_;
                boss_wait_cond_var_.notify_one();
            }
        }

    public:
        ThreadPoolBreakable(int thread_count) : ThreadPoolBase(thread_count) {}

        void WaitForBreakOrTerminate(bool &is_break) {
            if (left_tasks_counter_ > 0) {
                auto lock = make_unique_lock(boss_wait_mutex_);
                while (left_tasks_counter_ != 0 && !is_breaking_)
                    boss_wait_cond_var_.wait(lock);
            }
            is_break = is_breaking_;
            is_breaking_ = false;
        }
    };
}

#endif //CODES_YCHE_THREAD_POOL_BREAKABLE_H
