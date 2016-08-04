//
// Created by cheyulin on 8/1/16.
//

#ifndef CODES_YCHE_FIND_GRAINED_MERGE_SCHEDULER_H
#define CODES_YCHE_FIND_GRAINED_MERGE_SCHEDULER_H

#include <boost/range.hpp>
#include <memory>
#include <vector>
#include "semaphore.h"
#include <iostream>
#include <pthread.h>

#include "configuration.h"

using namespace std;

template<typename ReduceDataType, typename ComputationFuncType, typename ActionFuncType, typename FailActionFuncType>
class FineGrainedMergeScheduler {
public:
    FineGrainedMergeScheduler(unsigned long thread_count,
                              vector<unique_ptr<ReduceDataType>> &&reduce_data_ptr_vector,
                              ComputationFuncType pair_computation_func, ActionFuncType success_action_func,
                              FailActionFuncType fail_action_func) :
            thread_count_(thread_count), pair_computation_func_(
            pair_computation_func), success_action_func_(success_action_func), fail_action_func_(fail_action_func) {
        thread_handles_ = new pthread_t[thread_count_];
        reduce_data_ptr_vector_ = std::move(reduce_data_ptr_vector);
        sem_mail_boxes_.resize(thread_count_);
        for (auto i = 0; i < thread_count_; i++) {
            sem_init(&sem_mail_boxes_[i], 0, 0);
        }
        pthread_mutex_init(&terminate_in_advance_mutex_lock_, NULL);
        pthread_mutex_init(&counter_mutex_lock_, NULL);
        pthread_barrier_init(&timestamp_barrier_, NULL, thread_count_);

        local_computation_range_index_vec_.resize(thread_count_);
    }

    unique_ptr<ReduceDataType> Execute();

    virtual  ~FineGrainedMergeScheduler() {
        delete[]thread_handles_;
        for (auto i = 0; i < thread_count_; i++) {
            sem_destroy(&sem_mail_boxes_[i]);
        }
        pthread_mutex_destroy(&terminate_in_advance_mutex_lock_);
        pthread_mutex_destroy(&counter_mutex_lock_);
    }

private:
    struct BundleInput {
        FineGrainedMergeScheduler *fine_grained_merge_scheduler_ptr_;
        unsigned long thread_id_;
    };

    unsigned long thread_count_;
    unsigned long idle_count_;
    pthread_t *thread_handles_;

    vector<unique_ptr<ReduceDataType>> reduce_data_ptr_vector_;
    unsigned long right_reduce_data_index_;
    using ElementValueType = typename boost::range_value<ReduceDataType>::type;
    ElementValueType left_element_ptr_;

    pthread_barrier_t timestamp_barrier_;
    pthread_mutex_t terminate_in_advance_mutex_lock_;
    pthread_mutex_t counter_mutex_lock_;
    vector<sem_t> sem_mail_boxes_;

    vector<bool> is_rec_mail_empty_;
    bool is_terminate_in_advance_;
    bool is_end_of_loop_;

    vector<pair<long, long>> local_computation_range_index_vec_;

    ComputationFuncType pair_computation_func_;
    ActionFuncType success_action_func_;
    FailActionFuncType fail_action_func_;

    void InitInnerForLoopComputationTasks();

    void RingCommThreadFunction(unsigned long thread_id);

    static void *InvokeRingCommThreadFunction(void *bundle_input_ptr);

    inline void ResetStatesBeforeNextInnerForLoop();

    void ReduceComputation();
};

template<typename ReduceDataType, typename ComputationFuncType, typename ActionFuncType, typename FailActionFuncType>
void *FineGrainedMergeScheduler<ReduceDataType, ComputationFuncType,
        ActionFuncType, FailActionFuncType>::InvokeRingCommThreadFunction(void *bundle_input_ptr) {
    auto my_bundle_input_ptr = ((BundleInput *) bundle_input_ptr);
    my_bundle_input_ptr->fine_grained_merge_scheduler_ptr_->RingCommThreadFunction(
            my_bundle_input_ptr->thread_id_);
    return NULL;
}

template<typename ReduceDataType, typename ComputationFuncType, typename ActionFuncType, typename FailActionFuncType>
void FineGrainedMergeScheduler<ReduceDataType, ComputationFuncType,
        ActionFuncType, FailActionFuncType>::InitInnerForLoopComputationTasks() {
    //First, initialize indices
    auto whole_size = reduce_data_ptr_vector_[0]->size();
    auto avg_size = whole_size / thread_count_;
    for (auto i = 0; i < thread_count_ - 1; i++) {
        local_computation_range_index_vec_[i].first = avg_size * i;
        local_computation_range_index_vec_[i].second = avg_size * (i + 1) - 1;
    }
    local_computation_range_index_vec_[thread_count_ - 1].first = avg_size * (thread_count_ - 1);
    local_computation_range_index_vec_[thread_count_ - 1].second = whole_size - 1;
#ifdef DEBUG
    for (auto i = 0; i < thread_count_; i++) {
        cout << "Task Tid" << i << ":\t" << local_computation_range_index_vec_[i].first << ","
             << local_computation_range_index_vec_[i].second << endl;
    }
#endif
}

template<typename ReduceDataType, typename ComputationFuncType, typename ActionFuncType, typename FailActionFuncType>
void FineGrainedMergeScheduler<ReduceDataType, ComputationFuncType,
        ActionFuncType, FailActionFuncType>::ResetStatesBeforeNextInnerForLoop() {
    is_end_of_loop_ = false;
    is_terminate_in_advance_ = false;
    idle_count_ = 0;
    is_rec_mail_empty_.resize(thread_count_, true);
};

//Parallelize the inner for loop
template<typename ReduceDataType, typename ComputationFuncType, typename ActionFuncType, typename FailActionFuncType>
void FineGrainedMergeScheduler<ReduceDataType, ComputationFuncType,
        ActionFuncType, FailActionFuncType>::ReduceComputation() {
    std::vector<BundleInput *> input_bundle_vec(thread_count_);
    int round_num = 0;
    for (auto &left_element_ptr:*reduce_data_ptr_vector_[right_reduce_data_index_]) {
#ifdef DEBUG
        cout << "Round:" << (++round_num) << endl << endl;
#endif
        left_element_ptr_ = std::move(left_element_ptr);
        //Reset Value To Prepare for Next Iteration
        ResetStatesBeforeNextInnerForLoop();
        InitInnerForLoopComputationTasks();

        //Fork-join Boss-Worker Model
        for (auto thread_id = 0; thread_id < thread_count_; thread_id++) {
            input_bundle_vec[thread_id] = new BundleInput();
            input_bundle_vec[thread_id]->fine_grained_merge_scheduler_ptr_ = this;
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

        if (!is_terminate_in_advance_) {
            fail_action_func_(left_element_ptr_, reduce_data_ptr_vector_[0]);
        }
    }
}

template<typename ReduceDataType, typename ComputationFuncType, typename ActionFuncType, typename FailActionFuncType>
unique_ptr<ReduceDataType> FineGrainedMergeScheduler<ReduceDataType, ComputationFuncType,
        ActionFuncType, FailActionFuncType>::Execute() {
    while (reduce_data_ptr_vector_.size() > 1) {
        cout << "LeftReduceData:" << reduce_data_ptr_vector_.size() << endl;
        right_reduce_data_index_ = reduce_data_ptr_vector_.size() - 1;
        ReduceComputation();
//        cout << "Finish One" << endl;
        reduce_data_ptr_vector_.erase(reduce_data_ptr_vector_.end() - 1);
    }
    return std::move(reduce_data_ptr_vector_[0]);
}

template<typename ReduceDataType, typename ComputationFuncType, typename ActionFuncType, typename FailActionFuncType>
void FineGrainedMergeScheduler<ReduceDataType, ComputationFuncType,
        ActionFuncType, FailActionFuncType>::RingCommThreadFunction(
        unsigned long thread_id) {
    unsigned long thread_index = thread_id;
    auto dst_index = (thread_index + 1) % thread_count_;
    auto src_index = (thread_index - 1 + thread_count_) % thread_count_;
    auto &local_computation_data_indices = local_computation_range_index_vec_[thread_index];
    //Execute Tasks From end_index to start_index
    while (!is_end_of_loop_) {
        long computation_data_size =
                local_computation_data_indices.second - local_computation_data_indices.first + 1;
#pragma omp critical
        cout << "TID:" << thread_index << "\t" << local_computation_data_indices.first
             << "," << local_computation_data_indices.second << "," << computation_data_size << endl;
        if (computation_data_size == 0) {
            if (idle_count_ == thread_count_ - 1) {
                is_end_of_loop_ = true;
                for (auto i = 0; i < thread_count_; ++i) {
                    if (is_rec_mail_empty_[i] == false)
                        sem_post(&sem_mail_boxes_[i]);
                }
                break;
            }
            else {
                //Deal With the case that thread wants to wait for tasks after is_end_of_loop
                if (!is_end_of_loop_) {
                    pthread_mutex_lock(&counter_mutex_lock_);
                    if (idle_count_ == thread_count_ - 1) {
                        pthread_mutex_unlock(&counter_mutex_lock_);
                        is_end_of_loop_ = true;
                        for (auto i = 0; i < thread_count_; ++i) {
                            if (is_rec_mail_empty_[i] == false)
                                sem_post(&sem_mail_boxes_[i]);
                        }
                        break;
                    }
                    idle_count_++;
                    pthread_mutex_unlock(&counter_mutex_lock_);
                    is_rec_mail_empty_[dst_index] = false;
                    sem_wait(&sem_mail_boxes_[dst_index]);
                    if (is_end_of_loop_) {
                        break;
                    }
                    pthread_mutex_lock(&counter_mutex_lock_);
                    idle_count_--;
                    pthread_mutex_unlock(&counter_mutex_lock_);

                }
            }

        }
        else {
            if (computation_data_size > 1) {
                //Check Flag and Assign Tasks To Left Neighbor
                if (is_rec_mail_empty_[thread_index] == false) {
#pragma omp critical
                    cout << "Tid:" << thread_index << "\tReady Change:" << local_computation_data_indices.first << ","
                         << local_computation_data_indices.second << ","
                         << computation_data_size << "|"
                         << local_computation_data_indices.second - local_computation_data_indices.first + 1 << endl
                         << endl;

                    local_computation_range_index_vec_[src_index].second = local_computation_data_indices.second;
                    local_computation_range_index_vec_[src_index].first =
                            local_computation_data_indices.second - computation_data_size / 2 + 1;;
                    local_computation_data_indices.second = local_computation_range_index_vec_[src_index].first - 1;
                    is_rec_mail_empty_[thread_index] = true;
#ifdef DEBUG
#pragma omp critical
                    cout <<"Tid:"<<thread_index<< "\tUpdate Cur:" << local_computation_data_indices.first << ","
                         << local_computation_data_indices.second << ",Neighbor"
                         << local_computation_range_index_vec_[src_index].first << "," <<
                         local_computation_range_index_vec_[src_index].second << endl;
#endif
                    sem_post(&sem_mail_boxes_[thread_index]);
                }
            }
            auto &global_tasks_vec = *reduce_data_ptr_vector_[0];
            auto &right_element_ptr = global_tasks_vec[local_computation_data_indices.first];
            if (left_element_ptr_ == nullptr) {
#pragma omp critical
                cout << "Left element null" << endl;
            }
            if (right_element_ptr == nullptr) {
#pragma omp critical
                cout << "Right element null" << endl;
            }
            bool is_going_to_terminate = pair_computation_func_(left_element_ptr_, right_element_ptr);
            //Update Task Index
            local_computation_data_indices.first++;

            if (is_going_to_terminate) {
                pthread_mutex_lock(&terminate_in_advance_mutex_lock_);
                if (!is_terminate_in_advance_) {
                    is_terminate_in_advance_ = true;
                    is_end_of_loop_ = true;
                    //Do action, e.g, Merge left to right one
                    success_action_func_(left_element_ptr_, right_element_ptr);
                }
                pthread_mutex_unlock(&terminate_in_advance_mutex_lock_);
            }
        }
    }

    //Arouse waiting ones
    if (is_rec_mail_empty_[thread_index] == false) {
        sem_post(&sem_mail_boxes_[thread_index]);
    }

}


#endif //CODES_YCHE_FIND_GRAINED_MERGE_SCHEDULER_H
