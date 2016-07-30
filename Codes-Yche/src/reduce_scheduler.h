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
#include <algorithm>

#include "configuration.h"

namespace yche {
    using namespace std;

    template<typename IndexType>
    struct ReduceTaskIndices {
        IndexType result_index_;
        IndexType start_computation_index_;
        IndexType end_computation_index_;

        inline void InitTaskIndices(IndexType global_start_index, IndexType global_end_index) {
            result_index_ = global_start_index;
            start_computation_index_ = global_start_index + 1;
            end_computation_index_ = global_end_index;
        }

        inline void RescheduleTaskIndices(IndexType new_start_computation_index, IndexType new_end_computation_index) {
            start_computation_index_ = new_start_computation_index;
            end_computation_index_ = new_end_computation_index;
        }

        inline IndexType GetInitStartIndex() {
            return result_index_;
        }

        inline IndexType GetInitEndIndex() {
            return end_computation_index_;
        }

        inline IndexType GetReduceTaskSize() {
            return end_computation_index_ - start_computation_index_ + 1;
        }
    };

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
        pthread_t *thread_handles_;

        using IndexType = unsigned long;
        vector<unique_ptr<Data>> first_phase_reduce_data_pool_vec_;
        vector<unique_ptr<Data>> second_phase_global_reduce_data_vector;
        vector<ReduceTaskIndices<IndexType>> reduce_data_indices_vec_;
        vector<bool> is_rec_mail_empty_;

        vector<sem_t> sem_mail_boxes_;
        vector<pthread_mutex_t> check_indices_mutex_lock_vector_;
        pthread_mutex_t counter_mutex_lock_;
        pthread_mutex_t task_taking_mutex_;
        pthread_cond_t task_taking_cond_var_;
        pthread_barrier_t timestamp_barrier_;

        vector<bool> is_busy_working_;
        bool is_reduce_task_only_one_;
        bool is_end_of_loop_;
        bool is_end_of_reduce_;

        DataCmpFunction data_cmp_function_;
        ComputationFunction reduce_compute_function_;

        void RingCommTaskStealAndRequestThreadFunction(unsigned long thread_id);

        static void *InvokeRingCommThreadFunction(void *bundle_input_ptr);

        void InitDataPerThread(DataCollection &data_collection);

    public:

        unique_ptr<Data> ParallelExecute();

        Reducer(unsigned long thread_count, DataCollection &reduce_data_collection, DataCmpFunction data_cmp_function,
                ComputationFunction compute_function)
                : thread_count_(thread_count), data_cmp_function_(data_cmp_function),
                  reduce_compute_function_(compute_function) {
            thread_handles_ = new pthread_t[thread_count_];
            data_count_ = 0;

            sem_mail_boxes_.resize(thread_count_);
            for (auto i = 0; i < thread_count_; i++) {
                sem_init(&sem_mail_boxes_[i], 0, 0);
            }
            check_indices_mutex_lock_vector_.resize(thread_count_);
            for (auto i = 0; i < thread_count_; i++) {
                pthread_mutex_init(&check_indices_mutex_lock_vector_[i], NULL);
            }

            pthread_barrier_init(&timestamp_barrier_, NULL, thread_count_);
            pthread_mutex_init(&task_taking_mutex_, NULL);
            pthread_mutex_init(&counter_mutex_lock_, NULL);
            pthread_cond_init(&task_taking_cond_var_, NULL);
            reduce_data_indices_vec_.resize(thread_count_);


            is_end_of_loop_ = false;
            is_end_of_reduce_ = false;
            is_reduce_task_only_one_ = false;
            is_rec_mail_empty_.resize(thread_count_, true);
            is_busy_working_.resize(thread_count_, false);
            idle_count_ = 0;
            //InitLocalData
            InitDataPerThread(reduce_data_collection);
        }

        virtual ~Reducer() {
            sem_mail_boxes_.resize(thread_count_);
            for (auto i = 0; i < thread_count_; i++) {
                sem_destroy(&sem_mail_boxes_[i]);
            }
            for (auto i = 0; i < thread_count_; i++) {
                pthread_mutex_destroy(&check_indices_mutex_lock_vector_[i]);
            }
            pthread_mutex_destroy(&task_taking_mutex_);
            pthread_mutex_destroy(&counter_mutex_lock_);
            pthread_cond_destroy(&task_taking_cond_var_);
            pthread_barrier_destroy(&timestamp_barrier_);
            delete[]thread_handles_;
        }
    };

    template<typename DataCollection, typename Data, typename DataCmpFunction, typename ComputationFunction>
    void Reducer<DataCollection, Data, DataCmpFunction, ComputationFunction>::InitDataPerThread(
            DataCollection &data_collection) {
        data_count_ = data_collection.size();
        if (data_count_ == 1) {
            is_reduce_task_only_one_ = true;
            first_phase_reduce_data_pool_vec_.push_back(std::move(*data_collection.begin()));
            return;
        }
        first_phase_reduce_data_pool_vec_.resize(data_collection.size());
        int i = 0;
        for (auto &data:data_collection) {
            first_phase_reduce_data_pool_vec_[i] = std::move(data);
            i++;
        }
        auto task_per_thread = data_collection.size() / thread_count_;
        cout << task_per_thread << " tasks per thread" << endl;

        //Average Partitioning Tasks
        for (auto i = 0; i < thread_count_ - 1; ++i) {
            reduce_data_indices_vec_[i].InitTaskIndices(i * task_per_thread, (i + 1) * task_per_thread - 1);
        }
        reduce_data_indices_vec_[thread_count_ - 1].InitTaskIndices((thread_count_ - 1) * task_per_thread,
                                                                    data_collection.size() - 1);
        //Sort From Greatest to Least
        for (auto i = 0; i < thread_count_; i++) {
            sort(first_phase_reduce_data_pool_vec_.begin() + reduce_data_indices_vec_[i].GetInitStartIndex(),
                 first_phase_reduce_data_pool_vec_.begin() + reduce_data_indices_vec_[i].GetInitEndIndex(),
                 data_cmp_function_);
        }
        cout << "Reduce Task Init Finished" << endl;
#ifdef DEBUG
        for (auto i = 0; i < thread_count_; ++i) {
            cout << reduce_data_indices_vec_[i].result_index_ << ","
                 << reduce_data_indices_vec_[i].start_computation_index_ << ","
                 << reduce_data_indices_vec_[i].end_computation_index_ << endl;
        }
#endif
    }

    template<typename DataCollection, typename Data, typename DataCmpFunction, typename ComputationFunction>
    void Reducer<DataCollection, Data, DataCmpFunction, ComputationFunction>::RingCommTaskStealAndRequestThreadFunction(
            unsigned long thread_id) {
        unsigned long thread_index = thread_id;
        auto dst_index = (thread_index + 1) % thread_count_;
        auto src_index = (thread_index - 1 + thread_count_) % thread_count_;
        auto &local_reduce_data_indices = reduce_data_indices_vec_[thread_index];

        struct timespec begin, end;
        if (thread_index == 0) {
            clock_gettime(CLOCK_MONOTONIC, &begin);
        }
        if (thread_count_ < data_count_) {
            while (true) {
                auto reduce_data_size = local_reduce_data_indices.GetReduceTaskSize();
                if (reduce_data_size == 0) {
                    if (idle_count_ == thread_count_ - 1) {
                        is_end_of_loop_ = true;
                        idle_count_ = 0;
                        for (auto i = 0; i < thread_count_; ++i) {
                            if (i != dst_index)
                                sem_post(&sem_mail_boxes_[i]);
                        }
                        break;
                    }
                    else {
                        pthread_mutex_lock(&counter_mutex_lock_);
                        idle_count_++;
                        pthread_mutex_unlock(&counter_mutex_lock_);

                        bool is_going_to_request = false;

#ifdef STEAL_ENABLE
                        pthread_mutex_lock(&check_indices_mutex_lock_vector_[thread_index]);
                        pthread_mutex_lock(&check_indices_mutex_lock_vector_[dst_index]);
                        auto available_task_num = reduce_data_indices_vec_[dst_index].GetReduceTaskSize();
                        if (available_task_num == 0 ||
                            (available_task_num == 1 && is_busy_working_[dst_index] == true)) {
                            is_going_to_request = true;
                        } else {
                            if (is_busy_working_[dst_index]) {
                                //Index Should Consider Dst Current Computation
                                auto current_end_index = reduce_data_indices_vec_[dst_index].end_computation_index_-1;
                                auto current_start_index = current_end_index - (reduce_data_size + 1) / 2 + 1;
                                reduce_data_indices_vec_[thread_index].RescheduleTaskIndices(current_start_index,
                                                                                             current_end_index);
                                reduce_data_indices_vec_[dst_index].end_computation_index_=current_start_index;
                            }
                            else {
                                //Not Required to Consider Dst Current Computation, since it not enter critical section
                                auto current_end_index = reduce_data_indices_vec_[dst_index].end_computation_index_;
                                auto current_start_index = current_end_index - (reduce_data_size + 1) / 2 + 1;
                                reduce_data_indices_vec_[thread_index].RescheduleTaskIndices(current_start_index,
                                                                                             current_end_index);
                                reduce_data_indices_vec_[dst_index].end_computation_index_=current_start_index-1;
                            }
                            //Update Current Thread Index and Dst Index
                        }
                        pthread_mutex_unlock(&check_indices_mutex_lock_vector_[dst_index]);
                        pthread_mutex_unlock(&check_indices_mutex_lock_vector_[thread_index]);

                        if (is_going_to_request) {
                            is_rec_mail_empty_[dst_index] = false;
                            sem_wait(&sem_mail_boxes_[dst_index]);
                        }
#else
                        is_rec_mail_empty_[dst_index] = false;
                        sem_wait(&sem_mail_boxes_[dst_index]);
#endif
                        if (is_end_of_loop_) {
                            break;
                        }
                        pthread_mutex_lock(&counter_mutex_lock_);
                        idle_count_--;
                        pthread_mutex_unlock(&counter_mutex_lock_);
                    }
                }
                else {
                    if (reduce_data_size > 1) {
                        //Check Flag and Assign Tasks To Left Neighbor
                        if (is_rec_mail_empty_[thread_index] == false) {
                            auto neighbor_end_index = reduce_data_indices_vec_[thread_index].end_computation_index_;
                            auto neighbor_start_index = neighbor_end_index - reduce_data_size / 2 + 1;
                            reduce_data_indices_vec_[src_index].RescheduleTaskIndices(
                                    neighbor_start_index, neighbor_end_index);
                            local_reduce_data_indices.end_computation_index_ = neighbor_start_index - 1;
                            is_rec_mail_empty_[thread_index] = true;
                            sem_post(&sem_mail_boxes_[thread_index]);
                        }
                    }

                    //To Avoid Enter into Computation when the neighbor is stealing
#ifdef STEAL_ENABLE
                    pthread_mutex_lock(&check_indices_mutex_lock_vector_[src_index]);
                    is_busy_working_[thread_index] = true;
                    //Task Has been Steal
                    if (local_reduce_data_indices.GetReduceTaskSize() == 0)
                        continue;
                    pthread_mutex_unlock(&check_indices_mutex_lock_vector_[src_index]);
#endif
                    //Do reduce computation, use the first max one and the last min one
                    first_phase_reduce_data_pool_vec_[local_reduce_data_indices.result_index_] = std::move(
                            reduce_compute_function_(
                                    first_phase_reduce_data_pool_vec_[local_reduce_data_indices.result_index_],
                                    first_phase_reduce_data_pool_vec_[local_reduce_data_indices.end_computation_index_]));
#ifdef STEAL_ENABLE

                    pthread_mutex_lock(&check_indices_mutex_lock_vector_[thread_index]);
                    is_busy_working_[thread_index] = false;
                    local_reduce_data_indices.end_computation_index_--;
                    pthread_mutex_unlock(&check_indices_mutex_lock_vector_[thread_index]);
#else
                    local_reduce_data_indices.end_computation_index_--;
#endif
                }
            }
        }

        double elapsed;
        //Barrier
        pthread_barrier_wait(&timestamp_barrier_);

        if (thread_index == 0) {
            clock_gettime(CLOCK_MONOTONIC, &end);
            elapsed = end.tv_sec - begin.tv_sec;
            elapsed += (end.tv_nsec - begin.tv_nsec) / 1000000000.0;
            cout << "Before Global Variable Task Acq In First-Phase Reduce:" << elapsed << endl;
        }

#ifndef REDUCE_2ND_PHASE_SEQUENTIAL
        //Reduce Data Size Has become much larger in this phase, Maybe Need Fine-Grained Parallelism
        //Do left things, 1) send data back to global variable 2) use condition variable to synchronize
        unique_ptr<Data> result_data_ptr = std::move(
                first_phase_reduce_data_pool_vec_[reduce_data_indices_vec_[thread_index].result_index_]);
        unique_ptr<Data> input_data_ptr;
        pthread_barrier_wait(&timestamp_barrier_);
        cout << "Thread Index:" << thread_index << endl;

        while (!is_end_of_reduce_) {
            pthread_mutex_lock(&task_taking_mutex_);
            //Send result to global vector
            second_phase_global_reduce_data_vector.push_back(std::move(result_data_ptr));
            //Fetch Data : Busy Worker
            if (second_phase_global_reduce_data_vector.size() >= 2) {
                result_data_ptr = std::move(second_phase_global_reduce_data_vector.back());
                second_phase_global_reduce_data_vector.erase(second_phase_global_reduce_data_vector.end() - 1);
                input_data_ptr = std::move(second_phase_global_reduce_data_vector.back());
                second_phase_global_reduce_data_vector.erase(second_phase_global_reduce_data_vector.end() - 1);
                pthread_mutex_unlock(&task_taking_mutex_);

                //Do the computation After release the lock
                if (is_end_of_reduce_)
                    break;
                result_data_ptr = std::move(reduce_compute_function_(result_data_ptr, input_data_ptr));
            }
                //Judge Whether The Whole Computation Finished
            else {
                if (idle_count_ == thread_count_ - 1) {
                    is_end_of_reduce_ = true;
                    pthread_cond_broadcast(&task_taking_cond_var_);
                }
                else {
                    idle_count_++;
                    //Idle Worker, Go to Cond Var Buffer
                    while (pthread_cond_wait(&task_taking_cond_var_, &task_taking_mutex_) != 0);
                }
                pthread_mutex_unlock(&task_taking_mutex_);
            }
        }
#else
        //Send Back And Ask Corresponding One to Finish
        pthread_mutex_lock(&task_taking_mutex_);
        second_phase_global_reduce_data_vector.resize(thread_count_);
        pthread_mutex_unlock(&task_taking_mutex_);
        second_phase_global_reduce_data_vector[thread_index] = std::move(
                first_phase_reduce_data_pool_vec_[reduce_data_indices_vec_[thread_index].result_index_]);
        pthread_barrier_wait(&timestamp_barrier_);
        cout << "Finished Send Back 2 Global" << endl;
#endif
    }

    template<typename DataCollection, typename Data, typename DataCmpFunction, typename ComputationFunction>
    void *Reducer<DataCollection, Data, DataCmpFunction, ComputationFunction>::InvokeRingCommThreadFunction(
            void *bundle_input_ptr) {
        auto my_bundle_input_ptr = ((BundleInput *) bundle_input_ptr);
        my_bundle_input_ptr->reducer_ptr_->RingCommTaskStealAndRequestThreadFunction(my_bundle_input_ptr->thread_id_);
        return NULL;
    }

    template<typename DataCollection, typename Data, typename DataCmpFunction, typename ComputationFunction>
    unique_ptr<Data> Reducer<DataCollection, Data, DataCmpFunction, ComputationFunction>::ParallelExecute() {
        if (is_reduce_task_only_one_) {
            return std::move(first_phase_reduce_data_pool_vec_[0]);
        }
        else {
            std::vector<BundleInput *> input_bundle_vec(thread_count_);
            for (auto thread_id = 0; thread_id < thread_count_; thread_id++) {
                input_bundle_vec[thread_id] = new BundleInput();
                input_bundle_vec[thread_id]->reducer_ptr_ = this;
                input_bundle_vec[thread_id]->thread_id_ = thread_id;
                pthread_create(&thread_handles_[thread_id], NULL, this->InvokeRingCommThreadFunction,
                               (void *) input_bundle_vec[thread_id]);
            }

            for (auto thread_id = 0; thread_id < thread_count_; thread_id++) {
                pthread_join(thread_handles_[thread_id], NULL);
            }

            //Delete After All Execution-Flow Join
            for (auto i = 0; i < thread_count_; ++i) {
                delete input_bundle_vec[i];
            }

#ifdef REDUCE_2ND_PHASE_SEQUENTIAL
            cout << "Do Left Reduce In Single Core" << endl;
            //Do Left Reduce
            while (second_phase_global_reduce_data_vector.size() > 1) {
                cout << second_phase_global_reduce_data_vector.size() << endl;
                second_phase_global_reduce_data_vector[0] =
                        std::move(reduce_compute_function_(second_phase_global_reduce_data_vector[0],
                                                           second_phase_global_reduce_data_vector.back()));
                second_phase_global_reduce_data_vector.erase(second_phase_global_reduce_data_vector.end()-1);
            }

#endif
            return std::move(second_phase_global_reduce_data_vector[0]);
        }
    }
}

#endif //CODES_YCHE_REDUCER_H
