//
// Created by cheyulin on 8/8/16.
//

#ifndef CODES_YCHE_THREAD_POOL_BREAKABLE_H
#define CODES_YCHE_THREAD_POOL_BREAKABLE_H

#include "thread_pool_base.h"

using namespace std;
namespace yche {
    struct BreakWithCallBackRetType {
        bool is_break_{false};
        std::function<void(void)> call_back_function_object_{nullptr};

        BreakWithCallBackRetType(bool is_break, const std::function<void(void)> &call_back_function_object)
                : is_break_(is_break), call_back_function_object_(call_back_function_object) {
        }

        BreakWithCallBackRetType() = default;
    };

    enum class BreakStatus {
        NOT_BREAK,
        IS_BREAKING,
        FINISH_CALLBACK
    };

    class ThreadPoolBreakable : public ThreadPoolBase<BreakWithCallBackRetType> {
    private:
        atomic<BreakStatus> breaking_status_{BreakStatus::NOT_BREAK};

    protected:
        virtual void DoThreadFunction() override {
            while (!is_ready_finishing_) {
                auto task_function = NextTask();
                cout << "task left:" << left_tasks_counter_ << endl;
                if (task_function != nullptr) {
                    BreakWithCallBackRetType call_back_ret_obj = task_function();
                    if (call_back_ret_obj.is_break_) {
                        breaking_status_ = BreakStatus::IS_BREAKING;
                        if (breaking_status_ == BreakStatus::IS_BREAKING) {
                            {
                                auto lock = make_unique_lock(task_queue_mutex_);
                                left_tasks_counter_ -= task_queue_.size();
                                task_queue_.clear();
                            }
                            cout << "exec callback" << endl;
                            call_back_ret_obj.call_back_function_object_();
                        }
                    }
                    --left_tasks_counter_;
                } else {
                    cout << "shit" << endl;
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
        }

    };
}

#endif //CODES_YCHE_THREAD_POOL_BREAKABLE_H
