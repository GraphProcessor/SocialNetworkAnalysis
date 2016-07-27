//
// Created by cheyulin on 5/7/16.
//

#ifndef CODES_YCHE_REDUCER_H
#define CODES_YCHE_REDUCER_H

#include "semaphore.h"
#include <pthread.h>
#include <ctime>
#include <iostream>
#include <memory>
#include <vector>

#include "configuration.h"

namespace yche {
    using namespace std;

    template<typename DataCollection, typename Data, typename DataCmpFunction, typename ComputationFunction>
    class Reducer {
    private:

        struct BundleInput {
            Reducer *reducer_ptr_;
            unsigned long thread_id_;
        };

        unsigned long thread_count_;
        unsigned long data_count_;
        unsigned long idle_count_;
        pthread_barrier_t timestamp_barrier;

        pthread_t *thread_handles;

        vector<unique_ptr<Data>> global_reduce_data_vec_;
        vector<vector<unique_ptr<Data>>> local_data_vec_;
        vector<sem_t> sem_mail_boxes_;
        vector<bool> is_rec_mail_empty_;
        bool is_reduce_task_only_one;

        pthread_mutex_t counter_mutex_;
        pthread_mutex_t task_taking_mutex_;
        pthread_cond_t task_taking_cond_var;

        bool is_end_of_loop_;
        bool is_end_of_reduce_;

        DataCmpFunction data_cmp_function_;
        ComputationFunction reduce_compute_function_;

        void RingCommThreadFunction(unsigned long thread_id);

        static void *InvokeRingCommThreadFunction(void *bundle_input_ptr);

        void InitDataPerThread(DataCollection &data_collection);

    public:

        unique_ptr<Data> ParallelExecute();

        Reducer(unsigned long thread_count, DataCollection &reduce_data_collection, DataCmpFunction data_cmp_function,
                ComputationFunction compute_function)
                : thread_count_(thread_count), data_cmp_function_(data_cmp_function),
                  reduce_compute_function_(compute_function) {
            thread_handles = new pthread_t[thread_count_];
            data_count_ = 0;
            sem_mail_boxes_.resize(thread_count_);

            pthread_barrier_init(&timestamp_barrier, NULL, thread_count);

            for (auto i = 0; i < thread_count_; ++i) {
                sem_init(&sem_mail_boxes_[i], 0, 0);
            }

            pthread_mutex_init(&task_taking_mutex_, NULL);
            pthread_mutex_init(&counter_mutex_, NULL);
            pthread_cond_init(&task_taking_cond_var, NULL);
            local_data_vec_.resize(thread_count_);
            is_end_of_loop_ = false;
            is_end_of_reduce_ = false;
            is_reduce_task_only_one = false;
            is_rec_mail_empty_.resize(thread_count_, true);
            idle_count_ = 0;
            //InitLocalData
            InitDataPerThread(reduce_data_collection);
        }

        virtual ~Reducer() {
            for (auto i = 0; i < thread_count_; ++i) {
                sem_destroy(&sem_mail_boxes_[i]);
            }
            pthread_mutex_destroy(&task_taking_mutex_);
            pthread_mutex_destroy(&counter_mutex_);
            pthread_cond_destroy(&task_taking_cond_var);
            pthread_barrier_destroy(&timestamp_barrier);
            delete[]thread_handles;
        }
    };

    template<typename DataCollection, typename Data, typename DataCmpFunction, typename ComputationFunction>
    void Reducer<DataCollection, Data, DataCmpFunction, ComputationFunction>::InitDataPerThread(
            DataCollection &data_collection) {
        data_count_ = data_collection.size();
        if (data_count_ == 1) {
            is_reduce_task_only_one = true;
            global_reduce_data_vec_.push_back(std::move(*data_collection.begin()));
            return;
        }
        auto task_per_thread = data_collection.size() / thread_count_;
        cout << task_per_thread << " !!!task per thread" << endl;
        //Average Partitioning Tasks
        for (auto i = 0; i < thread_count_; ++i) {
            if (i == thread_count_ - 1) {
                for (auto iter = data_collection.begin() + task_per_thread * i;
                     iter != data_collection.end(); ++iter)
                    local_data_vec_[i].push_back(std::move((*iter)));
            }
            else {
                for (auto iter = data_collection.begin() + task_per_thread * i;
                     iter != data_collection.begin() + task_per_thread * (i + 1); ++iter)
                    local_data_vec_[i].push_back(std::move((*iter)));
            }
        }
#ifdef DEBUG
        cout << "Reduce Task Init Finished" << endl;
#endif
        //Sort From Greatest to Least
        for (auto i = 0; i < thread_count_; i++) {
            sort(local_data_vec_[i].begin(), local_data_vec_[i].end(), data_cmp_function_);
        }

    }

    template<typename DataCollection, typename Data, typename DataCmpFunction, typename ComputationFunction>
    void Reducer<DataCollection, Data, DataCmpFunction, ComputationFunction>::RingCommThreadFunction(
            unsigned long thread_id) {
        unsigned long thread_index = thread_id;
        auto dst_index = (thread_index + 1) % thread_count_;
        auto src_index = (thread_index - 1 + thread_count_) % thread_count_;
        auto &local_reduce_data_vec = local_data_vec_[thread_index];
        struct timespec begin, end;
        if (thread_index == 0) {
            clock_gettime(CLOCK_MONOTONIC, &begin);
        }
        if (thread_count_ < data_count_) {
            while (true) {
                auto reduce_data_size = local_reduce_data_vec.size();
//#ifdef DEBUG
//                cout << reduce_data_size << endl;
//#endif
                //Not able to conduct reduce operation
                if (reduce_data_size == 1) {
                    if (idle_count_ == thread_count_ - 1) {
                        is_end_of_loop_ = true;
                        //For Next Procedure Usage
                        idle_count_ = 0;
                        for (auto i = 0; i < thread_count_; ++i) {
                            if (i != dst_index)
                                sem_post(&sem_mail_boxes_[i]);
                        }
                        break;
                    }
                    else {
                        pthread_mutex_lock(&counter_mutex_);
                        idle_count_++;
                        pthread_mutex_unlock(&counter_mutex_);

                        is_rec_mail_empty_[dst_index] = false;
                        sem_wait(&sem_mail_boxes_[dst_index]);
                        if (is_end_of_loop_) {
                            break;
                        }
                        pthread_mutex_lock(&counter_mutex_);
                        idle_count_--;
                        pthread_mutex_unlock(&counter_mutex_);
                    }
                }

                else {
                    if (reduce_data_size > 2) {
                        //Check Flag
                        auto &neighbor_computation_queue = local_data_vec_[src_index];
                        if (is_rec_mail_empty_[thread_index] == false) {
                            for (auto iter = local_reduce_data_vec.begin() + local_reduce_data_vec.size() / 2;
                                 iter < local_reduce_data_vec.end(); ++iter) {
                                neighbor_computation_queue.push_back(std::move(*iter));
                                local_reduce_data_vec.erase(iter);
                            }
                            is_rec_mail_empty_[thread_index] = true;
                            sem_post(&sem_mail_boxes_[thread_index]);
                        }
                    }
                    //Do reduce computation, use the first max one and the last min one
                    local_reduce_data_vec[0] = std::move(reduce_compute_function_(local_reduce_data_vec[0],
                                                                                  local_reduce_data_vec.back()));
                    local_reduce_data_vec.erase(local_reduce_data_vec.end() - 1);
                }
            }
        }


        double elapsed;
        //Barrier
        pthread_barrier_wait(&timestamp_barrier);

        if (thread_index == 0) {
            clock_gettime(CLOCK_MONOTONIC, &end);
            elapsed = end.tv_sec - begin.tv_sec;
            elapsed += (end.tv_nsec - begin.tv_nsec) / 1000000000.0;
            cout << "Before Global Variable Task Acq In First-Phase Reduce:" << elapsed << endl;
        }

#ifndef REDUCE_2ND_PHASE_SEQUENTIAL
        //Reduce Data Size Has become much larger in this phase, Maybe Need Fine-Grained Parallelism
        //Do left things, 1) send data back to global variable 2) use condition variable to synchronize
        while (!is_end_of_reduce_) {
            pthread_mutex_lock(&task_taking_mutex_);
            //Send result to global vector
            for (auto &data_ptr: local_reduce_data_vec) {
                global_reduce_data_vec_.push_back(std::move(data_ptr));
            }
            local_reduce_data_vec.clear();

            //Fetch Data : Busy Worker
            if (global_reduce_data_vec_.size() >= 2) {
                for (auto i = 0; i < 2; i++) {
                    local_reduce_data_vec.push_back(std::move(global_reduce_data_vec_.back()));
                    global_reduce_data_vec_.erase(global_reduce_data_vec_.end() - 1);
                }
                pthread_mutex_unlock(&task_taking_mutex_);
                //Do the computation After release the lock
                if (is_end_of_reduce_)
                    break;
                local_reduce_data_vec[0] = std::move(reduce_compute_function_(local_reduce_data_vec[0],
                                                                              local_reduce_data_vec[1]));
                local_reduce_data_vec.resize(1);
            }
                //Judge Whether The Whole Computation Finished
            else {
                if (idle_count_ == thread_count_ - 1) {
                    is_end_of_reduce_ = true;
                    pthread_cond_broadcast(&task_taking_cond_var);
                }
                else {
                    idle_count_++;
                    //Idle Worker, Go to Cond Var Buffer
                    while (pthread_cond_wait(&task_taking_cond_var, &task_taking_mutex_) != 0);
                }
                pthread_mutex_unlock(&task_taking_mutex_);

            }
        }
#endif

        //Send Back And Ask Corresponding One to Finish
#ifdef REDUCE_2ND_PHASE_SEQUENTIAL
        pthread_mutex_lock(&task_taking_mutex_);
        global_reduce_data_vec_.resize(thread_count_);
        pthread_mutex_unlock(&task_taking_mutex_);
        global_reduce_data_vec_[thread_index]=std::move(local_reduce_data_vec[0]);
#endif
    }

    template<typename DataCollection, typename Data, typename DataCmpFunction, typename ComputationFunction>
    void *Reducer<DataCollection, Data, DataCmpFunction, ComputationFunction>::InvokeRingCommThreadFunction(
            void *bundle_input_ptr) {
        auto my_bundle_input_ptr = ((BundleInput *) bundle_input_ptr);
        my_bundle_input_ptr->reducer_ptr_->RingCommThreadFunction(my_bundle_input_ptr->thread_id_);
        return NULL;
    }

    template<typename DataCollection, typename Data, typename DataCmpFunction, typename ComputationFunction>
    unique_ptr<Data> Reducer<DataCollection, Data, DataCmpFunction, ComputationFunction>::ParallelExecute() {
        if (is_reduce_task_only_one) {
            return std::move(global_reduce_data_vec_[0]);
        }
        vector<BundleInput *> input_bundle_vec(thread_count_);
        for (auto thread_id = 0; thread_id < thread_count_; thread_id++) {
            input_bundle_vec[thread_id] = new BundleInput();
            input_bundle_vec[thread_id]->reducer_ptr_ = this;
            input_bundle_vec[thread_id]->thread_id_ = thread_id;
            pthread_create(&thread_handles[thread_id], NULL, this->InvokeRingCommThreadFunction,
                           (void *) input_bundle_vec[thread_id]);
        }

        for (auto thread_id = 0; thread_id < thread_count_; thread_id++) {
            pthread_join(thread_handles[thread_id], NULL);
        }

        //Delete After All Execution-Flow Join
        for (auto i = 0; i < thread_count_; ++i) {
            delete input_bundle_vec[i];
        }

#ifdef REDUCE_2ND_PHASE_SEQUENTIAL
        //Do Left Reduce
        auto global_reduce_data_vec_size =global_reduce_data_vec_.size();
        while(global_reduce_data_vec_size>1){
            global_reduce_data_vec_[global_reduce_data_vec_size-2]=
                    std::move(reduce_compute_function_(global_reduce_data_vec_[global_reduce_data_vec_size-1],
                                                       global_reduce_data_vec_[global_reduce_data_vec_size-2]));
            global_reduce_data_vec_size--;
        }
#endif
        return std::move(global_reduce_data_vec_[0]);


    }


}


#endif //CODES_YCHE_REDUCER_H
