//
// Created by cheyulin on 8/8/16.
//

#ifndef CODES_YCHE_THREAD_POOL_BREAKABLE_H
#define CODES_YCHE_THREAD_POOL_BREAKABLE_H

#include "thread_pool_base.h"

using namespace boost;
using std::cout;
using std::endl;

namespace yche {
    struct BreakWithCallBackRetType {
        bool is_break_{false};
        std::function<void(void)> call_back_function_object_{nullptr};

        BreakWithCallBackRetType(bool is_break, const std::function<void(void)> &call_back_function_object)
                : is_break_(is_break), call_back_function_object_(call_back_function_object) {
        }

        BreakWithCallBackRetType() = default;
    };

    class ThreadPoolBreakable : public ThreadPoolBase<BreakWithCallBackRetType> {
    private:
        atomic_bool is_break_{false};

    protected:
        virtual void DoThreadFunction() override {
            while (!is_ready_finishing_) {
                {
                    auto lock = make_unique_lock(task_queue_mutex_);
                    task_available_cond_var_.wait(lock, [this]() { return !is_break_; });
                }
                auto task_function = NextTask();
                if (task_function != nullptr) {
                    BreakWithCallBackRetType call_back_ret_obj = task_function();
                    if (call_back_ret_obj.is_break_) {
                        auto task_lock = make_unique_lock(task_queue_mutex_);
                        if (!is_break_) {
                            is_break_ = true;
                            left_tasks_counter_ = 0;
                            task_queue_.clear();
                            call_back_ret_obj.call_back_function_object_();
                        }
                    }
                    --left_tasks_counter_;
                }
                boss_wait_cond_var_.notify_one();
            }
        }

    public:
        ThreadPoolBreakable(int thread_count) : ThreadPoolBase(thread_count) {}

        virtual void AddTask(std::function<BreakWithCallBackRetType()> task) override {
            auto lock = make_unique_lock(task_queue_mutex_);
            if (!is_break_) {
                task_queue_.emplace_back(task);
                ++left_tasks_counter_;
                task_available_cond_var_.notify_one();
            }
        }

        void WaitForBreakOrTerminate(bool &is_break) {
            auto lock = make_unique_lock(boss_wait_mutex_);
            while (left_tasks_counter_ != 0) {
//                if (left_tasks_counter_ < 0) {
//                    left_tasks_counter_ = 0;
//                    break;
//                }
                boss_wait_cond_var_.wait(lock);
            }
            is_break = is_break_;
            is_break_ = false;
            if (is_break)
                task_available_cond_var_.notify_all();
        }
    };
}

#endif //CODES_YCHE_THREAD_POOL_BREAKABLE_H
