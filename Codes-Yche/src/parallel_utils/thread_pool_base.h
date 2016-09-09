//
// Created by cheyulin on 8/8/16.
//

#ifndef CODES_YCHE_THREAD_POOL_H
#define CODES_YCHE_THREAD_POOL_H

#include <boost/thread.hpp>
#include <boost/thread/lock_factories.hpp>
#include <list>

using namespace boost;
using std::list;

namespace yche {
    template<typename ResultType=void>
    class ThreadPoolBase {
    private:
        thread_group thread_list_;

    protected:
        list<std::function<ResultType(void)>> task_queue_;
        atomic_int left_tasks_counter_{0};
        atomic_bool is_ready_finishing_{false};
        atomic_bool is_finished_{false};

        mutex task_queue_mutex_;
        mutex boss_wait_mutex_;

        condition_variable_any task_available_cond_var_;
        condition_variable_any boss_wait_cond_var_;

        virtual void DoThreadFunction() {
            while (!is_ready_finishing_) {
                auto task_function = NextTask();
                if (task_function != nullptr) {
                    task_function();
                    --left_tasks_counter_;
                }
                boss_wait_cond_var_.notify_one();
            }
        }

        std::function<ResultType(void)> NextTask() {
            std::function<ResultType(void)> resource_function_object;
            auto lock = make_unique_lock(task_queue_mutex_);
            while (task_queue_.size() == 0 && !is_ready_finishing_)
                task_available_cond_var_.wait(lock);
            if (!is_ready_finishing_) {
                resource_function_object = task_queue_.front();
                task_queue_.pop_front();
            } else {
                resource_function_object = nullptr;
            }
            return resource_function_object;
        }

        void WaitAll() {
            while (left_tasks_counter_ != 0) {
                auto lock = make_unique_lock(boss_wait_mutex_);
                boss_wait_cond_var_.wait(lock);
            }
        }

        void JoinAll() {
            if (!is_finished_) {
                WaitAll();
                is_ready_finishing_ = true;
                task_available_cond_var_.notify_all();
                thread_list_.join_all();
                is_finished_ = true;
            }
        }

    public:
        ThreadPoolBase(int thread_count) {
            for (auto i = 0; i < thread_count; i++) {
                thread_list_.create_thread([this, i]() { this->DoThreadFunction(); });
            }
        }

        virtual ~ThreadPoolBase() {
            JoinAll();
        }

        size_t Size() const {
            return thread_list_.size();
        }

        size_t TasksRemaining() {
            auto lock = make_unique_lock(task_queue_mutex_);
            return task_queue_.size();
        }

        virtual void AddTask(std::function<ResultType(void)> task) {
            auto lock = make_unique_lock(task_queue_mutex_);
            task_queue_.emplace_back(std::move(task));
            ++left_tasks_counter_;
            task_available_cond_var_.notify_one();
        }
    };
}

#endif //CODES_YCHE_THREAD_POOL_H
