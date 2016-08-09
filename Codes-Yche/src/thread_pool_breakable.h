//
// Created by cheyulin on 8/8/16.
//

#ifndef CODES_YCHE_THREAD_POOL_BREAKABLE_H
#define CODES_YCHE_THREAD_POOL_BREAKABLE_H

#include "thread_pool_base.h"

using namespace std;
namespace yche {
    /*
     * traits: if NextTask() return true, break
     * BreakWithCallBackRetType is designed for callback
     */
    struct BreakWithCallBackRetType {
        bool is_break_{false};
        std::function<void(void)> call_back_function_object_{nullptr};

        BreakWithCallBackRetType(bool is_break, const std::function<void(void)> &call_back_function_object)
                : is_break_(is_break), call_back_function_object_(call_back_function_object) {}

        BreakWithCallBackRetType() = default;

        explicit operator bool() const {
            return is_break_;
        }
    };

    class ThreadPoolBreakable : public ThreadPoolBase<BreakWithCallBackRetType> {
    private:
        //introduced for break current phase computation and go into another one
        atomic_bool is_breaking_{false};

    protected:
        virtual void DoThreadFunction() override {
            while (!is_ending_) {
                auto task_function = NextTask();
                BreakWithCallBackRetType *call_back_ret_type_ptr = nullptr;
                //not bailout
                if (task_function != nullptr) {
                    *call_back_ret_type_ptr = task_function();
                    if (call_back_ret_type_ptr->is_break_) {
                        is_breaking_ = true;
                        cout << "!!!!!!!" << endl;
                        if (is_breaking_) {
                            {
                                auto lock = make_unique_lock(task_queue_mutex_);
                                left_tasks_counter_ -= task_queue_.size();
                                task_queue_.clear();
                            }
                            call_back_ret_type_ptr->call_back_function_object_();
                        }
                    }
                    --left_tasks_counter_;
                }
                boss_wait_cond_var_.notify_one();
            }
        }

    public:
        ThreadPoolBreakable(int thread_count) : ThreadPoolBase(thread_count) {}

        void WaitForBreakOrTerminate(bool &is_break) {
            if (left_tasks_counter_ > 0) {
                auto lock = make_unique_lock(boss_wait_mutex_);
                while (left_tasks_counter_ != 0)
                    boss_wait_cond_var_.wait(lock);
            }
            is_break = is_breaking_;
            is_breaking_ = false;
        }
    };
}

#endif //CODES_YCHE_THREAD_POOL_BREAKABLE_H
