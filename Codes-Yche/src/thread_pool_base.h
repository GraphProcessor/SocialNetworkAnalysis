//
// Created by cheyulin on 8/8/16.
//

#ifndef CODES_YCHE_THREAD_POOL_H
#define CODES_YCHE_THREAD_POOL_H

#include <boost/thread.hpp>
#include <boost/thread/lock_factories.hpp>
#include <list>

using namespace boost;
namespace yche {
    template<typename ResultType=void>
    class ThreadPoolBase {
    private:
        thread_group thread_list_;

    protected:
        std::list<std::function<ResultType(void)>> task_queue_;
        atomic_int left_tasks_counter_{0};
        atomic_bool is_go_ending_{false};
        atomic_bool is_finished_{false};

        mutex task_queue_mutex_;
        mutex boss_wait_mutex_;

        condition_variable_any task_available_cond_var_;
        condition_variable_any boss_wait_cond_var_;

        virtual void DoThreadFunction() {
            while (!is_go_ending_) {
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
            while (task_queue_.size() == 0 && !is_go_ending_)
                task_available_cond_var_.wait(lock);
            if (!is_go_ending_) {
                resource_function_object = task_queue_.front();
                task_queue_.pop_front();
            }
            else {
                resource_function_object = nullptr;
            }
            return resource_function_object;
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

        inline unsigned Size() const {
            return thread_list_.size();
        }

        inline unsigned TasksRemaining() {
            auto lock = make_unique_lock(task_queue_mutex_);
            return task_queue_.size();
        }

        void AddTask(std::function<ResultType(void)> task) {
            auto lock = make_unique_lock(task_queue_mutex_);
            task_queue_.emplace_back(std::move(task));
            ++left_tasks_counter_;
            task_available_cond_var_.notify_one();
        }

        void WaitAll() {
            if (left_tasks_counter_ > 0) {
                auto lock = make_unique_lock(boss_wait_mutex_);
                while (left_tasks_counter_ != 0)
                    boss_wait_cond_var_.wait(lock);
            }
        }

        void JoinAll(bool is_wait_for_all = true) {
            if (!is_finished_) {
                if (is_wait_for_all) {
                    WaitAll();
                }
                is_go_ending_ = true;
                task_available_cond_var_.notify_all();
                thread_list_.join_all();
                is_finished_ = true;
            }
        }
    };
}

#endif //CODES_YCHE_THREAD_POOL_H
