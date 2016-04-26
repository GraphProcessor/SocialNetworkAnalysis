//
// Created by cheyulin on 4/25/16.
//

#ifndef CODES_YCHE_PARALLELIZER_H
#define CODES_YCHE_PARALLELIZER_H

#include "semaphore.h"
#include <pthread.h>
#include "include_header.h"

namespace yche {
    using namespace std;
    enum class function_type {
        local_computation,
        merge
    };

    template<typename Data>
    struct Task {
        unique_ptr<Data> data_ptr_;

        Task(unique_ptr<Data> data) {
            data_ptr_ = std::move(data);
        }
    };


    template<typename Algorithm>
    class Parallelizer {
        struct BundleInput {
            Parallelizer *parallelizer_ptr_;
            unsigned long thread_id_;
        };

    private:
        unsigned long thread_count_;
        unsigned long idle_count_;

        using Data = typename Algorithm::BasicData;
        using MergeData = typename Algorithm::MergeData;
//        unique_ptr<vector<unique_ptr<Task<Data>>>> basic_tasks_ptr_;

        pthread_t *thread_handles;
        vector<vector<unique_ptr<Task<Data>>>> local_computation_task_queues_;
        vector<vector<unique_ptr<Task<MergeData>>>> merge_task_queues_;
        vector<sem_t> sem_mail_boxes_;
        sem_t sem_counter_;
        pthread_mutex_t merge_mutex_;

        vector<bool> is_rec_mail_empty_;

        bool is_any_mergeing;
        unique_ptr<Algorithm> algorithm_ptr_;

        void LoopCommThreadFunction(long thread_id);

        static void *InvokeLoopCommThreadFunction(void *bundle_input_ptr);

        void InitTasks();

    public:
        void ParallelExecute();

        Parallelizer(unsigned long thread_count,
                     unique_ptr<Algorithm> algorithm_ptr)
                : thread_count_(thread_count) {
            algorithm_ptr_ = std::move(algorithm_ptr);
            thread_handles = new pthread_t[thread_count_];
            sem_mail_boxes_.resize(thread_count_);
            for (auto i = 0; i < thread_count_; ++i) {
                sem_init(&sem_mail_boxes_[i], NULL, 0);
            }
            sem_init(&sem_counter_, NULL, 1);
            pthread_mutex_init(&merge_mutex_, NULL);
            is_rec_mail_empty_.resize(thread_count_, true);
            merge_task_queues_.resize(thread_count_);

            is_any_mergeing = false;
        }


        virtual ~Parallelizer() {

            for (auto i = 0; i < thread_count_; ++i) {
                sem_destroy(&sem_mail_boxes_[i]);
            }
            sem_destroy(&sem_counter_);
            pthread_mutex_destroy(&merge_mutex_);
            delete[]thread_handles;
        }
    };


    template<typename Algorithm>
    void Parallelizer<Algorithm>::ParallelExecute() {
        struct timespec begin, end;
        double elapsed;
        clock_gettime(CLOCK_MONOTONIC, &begin);

        InitTasks();
        BundleInput my_bundle_input;
        my_bundle_input.parallelizer_ptr_ = this;
        for (auto thread_id = 0; thread_id < thread_count_; thread_id++) {
            my_bundle_input.thread_id_ = thread_id;
            BundleInput *my_bundle_input_ptr = &my_bundle_input;
            pthread_create(&thread_handles[thread_id], NULL, this->InvokeLoopCommThreadFunction,
                           (void *) my_bundle_input_ptr);
        }

        for (auto thread_id = 0; thread_id < thread_count_; thread_id++) {
            pthread_join(thread_handles[thread_id], NULL);
        }
        clock_gettime(CLOCK_MONOTONIC, &end);
        elapsed = end.tv_sec - begin.tv_sec;
        elapsed += (end.tv_nsec - begin.tv_nsec) / 1000000000.0;
        cout << "Elapsed Time " << elapsed << endl;
    }

    template<typename Algorithm>
    void Parallelizer<Algorithm>::InitTasks() {
        auto basic_data_vec_ptr = algorithm_ptr_->InitBasicComputationData();
        auto basic_tasks_ptr = make_unique<vector<unique_ptr<Task<Data>>>>();
        for (auto &&basic_data_ptr:*basic_data_vec_ptr) {
            basic_tasks_ptr->push_back(make_unique<Task<Data>>(std::move(basic_data_ptr)));
        }
        auto task_per_thread = basic_tasks_ptr->size() / thread_count_;
        //Average Partitioning Tasks
        for (auto i = 0; i < thread_count_; ++i) {
            if (i == thread_count_ - 1) {
                for (auto iter = basic_tasks_ptr->begin() + task_per_thread * i;
                     iter != basic_tasks_ptr->end(); ++iter)
                    local_computation_task_queues_[i].push_back(std::move((*iter)->data_ptr_));
            }
            else {
                for (auto iter = basic_tasks_ptr->begin() + task_per_thread * i;
                     iter != basic_tasks_ptr->begin() + task_per_thread * (i + 1); ++iter)
                    local_computation_task_queues_[i].push_back(std::move((*iter)->data_ptr_));
            }
        }
    }

    template<typename Algorithm>
    void Parallelizer<Algorithm>::LoopCommThreadFunction(long thread_id) {
        unsigned long thread_index = (unsigned long) thread_id;
        auto dest_index = (thread_index + 1) % thread_count_;
        auto src_index = (thread_index - 1 + thread_count_) % thread_count_;
        auto &local_computation_queue = local_computation_task_queues_[thread_index];
        auto &local_merge_queue = merge_task_queues_[thread_index];

        while (true) {
            auto local_computation_task_size = local_computation_task_queues_[thread_index].size();
            if (local_computation_task_size == 0) {
                if (idle_count_ == thread_count_ - 1) {
                    break;
                }

                sem_wait(&sem_mail_boxes_[dest_index]);
                sem_wait(&sem_counter_);
                idle_count_++;
                sem_post(&sem_counter_);
            }

            else {
                if (local_computation_task_size > 1) {
                    //Check Flag
                    auto &neighbor_computation_queue = local_computation_task_queues_[src_index];
                    if (is_rec_mail_empty_[thread_index] == false) {
                        for (auto iter = local_computation_queue.begin() + local_computation_queue.size() / 2;
                             iter < local_computation_queue.end(); ++iter) {
                            neighbor_computation_queue.push_back(std::move(*iter));
                            local_computation_queue.erase(iter);
                        }
                        is_rec_mail_empty_[thread_index] = true;

                        sem_post(&sem_mail_boxes_[thread_index]);
                    }

                    //Do Local Computation
                    auto result = algorithm_ptr_->LocalComputation(
                            std::move(local_computation_queue.front()->data_ptr_));
                    local_computation_queue.erase(local_computation_queue.begin());

                    if (is_any_mergeing) {
                        local_merge_queue.push_back(std::move(make_unique<Task<MergeData>>(std::move(result))));
                    }
                    else {
                        pthread_mutex_lock(&merge_mutex_);
                        is_any_mergeing = true;

                        algorithm_ptr_->MergeToGlobal(std::move(result));
                        while (local_merge_queue.size() > 0) {
                            auto &&data = std::move(local_merge_queue.front()->data_ptr_);
                            algorithm_ptr_->MergeToGlobal(std::move(data));
                            local_merge_queue.erase(local_merge_queue.begin());
                        }
                        is_any_mergeing = false;
                        pthread_mutex_unlock(&merge_mutex_);
                    }
                }
            }
        }

        //Do Left Merging
        pthread_mutex_lock(&merge_mutex_);
        while (local_merge_queue.size() > 0) {
            auto &&data = std::move(local_merge_queue.front()->data_ptr_);
            algorithm_ptr_->MergeToGlobal(std::move(data));
            local_merge_queue.erase(local_merge_queue.begin());
        }
        pthread_mutex_unlock(&merge_mutex_);
    }

    template<typename Algorithm>
    void *Parallelizer<Algorithm>::InvokeLoopCommThreadFunction(void *bundle_input_ptr) {
        auto my_bundle_input = ((BundleInput *) bundle_input_ptr);
        my_bundle_input->parallelizer_ptr_->LoopCommThreadFunction(my_bundle_input->thread_id_);
    }

}


#endif //CODES_YCHE_PARALLELIZER_H
