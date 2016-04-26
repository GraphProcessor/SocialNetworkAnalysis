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
        unsigned long barrier_count_;

        using Data = typename Algorithm::BasicData;
        using MergeData = typename Algorithm::MergeData;
//        unique_ptr<vector<unique_ptr<Task<Data>>>> basic_tasks_ptr_;

        pthread_t *thread_handles;
        vector<vector<unique_ptr<Task<Data>>>> local_computation_task_vecs_;
        vector<vector<unique_ptr<Task<MergeData>>>> merge_task_vecs_;
        vector<sem_t> sem_mail_boxes_;
        sem_t sem_barrier_;
        sem_t sem_counter_;
        sem_t sem_barrier_counter_;

        pthread_mutex_t merge_mutex_;

        vector<bool> is_rec_mail_empty_;

        bool is_any_mergeing;

        bool is_end_of_local_computation;

        void LoopCommThreadFunction(unsigned long thread_id);

        static void *InvokeLoopCommThreadFunction(void *bundle_input_ptr);

        void InitTasks();

    public:
        unique_ptr<Algorithm> algorithm_ptr_;

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
            sem_init(&sem_barrier_, NULL, 0);
            sem_init(&sem_barrier_counter_, NULL, 1);

            pthread_mutex_init(&merge_mutex_, NULL);

            is_rec_mail_empty_.resize(thread_count_, true);
            local_computation_task_vecs_.resize(thread_count_);
            merge_task_vecs_.resize(thread_count_);

            is_end_of_local_computation = false;
            is_any_mergeing = false;

            idle_count_ = 0;
            barrier_count_ = 0;
        }


        virtual ~Parallelizer() {
//            cout <<"destroy"<<endl;
            for (auto i = 0; i < thread_count_; ++i) {
                sem_destroy(&sem_mail_boxes_[i]);
            }

            sem_destroy(&sem_counter_);
            sem_destroy(&sem_barrier_);
            sem_destroy(&sem_barrier_counter_);
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
        cout << "Finish Init" << endl;
        cout << local_computation_task_vecs_.size() << endl;
        vector<BundleInput *> input_bundle_vec(thread_count_);
        cout << "Thread_count:" << thread_count_ << endl;
        for (auto thread_id = 0; thread_id < thread_count_; thread_id++) {
            input_bundle_vec[thread_id] = new BundleInput();
            input_bundle_vec[thread_id]->parallelizer_ptr_ = this;
            input_bundle_vec[thread_id]->thread_id_ = thread_id;
//            cout << input_bundle_vec[thread_id]->thread_id_ << endl;
            pthread_create(&thread_handles[thread_id], NULL, this->InvokeLoopCommThreadFunction,
                           (void *) input_bundle_vec[thread_id]);
        }


        for (auto thread_id = 0; thread_id < thread_count_; thread_id++) {
            pthread_join(thread_handles[thread_id], NULL);
        }
        for (auto i = 0; i < thread_count_; ++i) {
            delete input_bundle_vec[i];
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
                    local_computation_task_vecs_[i].push_back(std::move((*iter)));
            }
            else {
                for (auto iter = basic_tasks_ptr->begin() + task_per_thread * i;
                     iter != basic_tasks_ptr->begin() + task_per_thread * (i + 1); ++iter)
                    local_computation_task_vecs_[i].push_back(std::move((*iter)));
            }
        }
    }

    template<typename Algorithm>
    void Parallelizer<Algorithm>::LoopCommThreadFunction(unsigned long thread_id) {
        struct timespec begin, end;
        double elapsed;
        clock_gettime(CLOCK_MONOTONIC, &begin);

        unsigned long thread_index = thread_id;
        auto dest_index = (thread_index + 1) % thread_count_;
        auto src_index = (thread_index - 1 + thread_count_) % thread_count_;
        auto &local_computation_queue = local_computation_task_vecs_[thread_index];
        auto &local_merge_queue = merge_task_vecs_[thread_index];
        while (true) {

            auto local_computation_task_size = local_computation_queue.size();
//            cout << local_computation_task_size << endl;
            if (local_computation_task_size == 0) {
                cout << thread_index << "  " << "I am idle " << endl;
                cout << idle_count_ << " " << "Idle Count" << endl;
                if (idle_count_ == thread_count_ - 1) {
                    is_end_of_local_computation = true;
                    for (auto i = 0; i < thread_count_; ++i) {
                        if (i != dest_index)
                            sem_post(&sem_mail_boxes_[i]);
                    }
                    cout << "Thread Finish!!!  " << thread_index << endl;
                    break;
                } else {
                    sem_wait(&sem_counter_);
                    idle_count_++;
                    sem_post(&sem_counter_);

                    is_rec_mail_empty_[dest_index] = false;
                    sem_wait(&sem_mail_boxes_[dest_index]);
                    if (is_end_of_local_computation) {
                        cout << "Thread Finish!!!  " << thread_index << endl;
                        break;
                    }
                    cout << "Thread Awake!!!  " << thread_index << endl;
                    sem_wait(&sem_counter_);
                    idle_count_--;
                    sem_post(&sem_counter_);
                }

            }

            else {
                if (local_computation_task_size > 1) {
                    //Check Flag
                    auto &neighbor_computation_queue = local_computation_task_vecs_[src_index];
                    if (is_rec_mail_empty_[thread_index] == false) {
                        for (auto iter = local_computation_queue.begin() + local_computation_queue.size() / 2;
                             iter < local_computation_queue.end(); ++iter) {
                            neighbor_computation_queue.push_back(std::move(*iter));
                            local_computation_queue.erase(iter);
                        }
                        is_rec_mail_empty_[thread_index] = true;

                        sem_post(&sem_mail_boxes_[thread_index]);
                    }
                }

                //Do Local Computation
                auto result = algorithm_ptr_->LocalComputation(
                        std::move(local_computation_queue.front()->data_ptr_));
                local_computation_queue.erase(local_computation_queue.begin());
//                cout << "Thread " << thread_index << ",My remaining work:" << local_computation_queue.size() << endl;
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

        //Barrier
        sem_wait(&sem_barrier_counter_);
        if (barrier_count_ == thread_count_ - 1) {
            barrier_count_ = 0;
            sem_post(&sem_barrier_counter_);
            for (auto thread = 0; thread < thread_count_ - 1; thread++) {
                sem_post(&sem_barrier_);
            }
        }
        else {
            barrier_count_++;
            sem_post(&sem_barrier_counter_);
            sem_wait(&sem_barrier_);
        }


        clock_gettime(CLOCK_MONOTONIC, &end);
        elapsed = end.tv_sec - begin.tv_sec;
        elapsed += (end.tv_nsec - begin.tv_nsec) / 1000000000.0;
        if (thread_index == 0) {
            cout << "Elpased Time In Parallel Computation:" << elapsed << endl;
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
        auto my_bundle_input_ptr = ((BundleInput *) bundle_input_ptr);
        my_bundle_input_ptr->parallelizer_ptr_->LoopCommThreadFunction(my_bundle_input_ptr->thread_id_);
    }

}


#endif //CODES_YCHE_PARALLELIZER_H
