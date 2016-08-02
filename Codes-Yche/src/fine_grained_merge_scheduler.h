//
// Created by cheyulin on 8/1/16.
//

#ifndef CODES_YCHE_FIND_GRAINED_MERGE_SCHEDULER_H
#define CODES_YCHE_FIND_GRAINED_MERGE_SCHEDULER_H

#include <boost/range.hpp>
#include <memory>
#include <vector>
#include "semaphore.h"
#include <pthread.h>

using namespace std;

template<typename ReduceDataType, typename ComputationFuncType, typename ActionFuncType, typename FailActionFuncType>
class FineGrainedMergeScheduler {
public:
    unique_ptr<ReduceDataType> Execute();

private:
    struct BundleInput {
        FineGrainedMergeScheduler *fine_grained_merge_scheduler_ptr_;
        unsigned long thread_id_;
    };

    unsigned long thread_count_;
    pthread_t *thread_handles_;

    vector<unique_ptr<ReduceDataType>> reduce_data_ptr_vector_;
    unsigned long right_reduce_data_index_;
    using ElementValueType = typename boost::range_value<ReduceDataType>::type;
    ElementValueType left_element_ptr_;


    pthread_mutex_t terminate_in_advance_mutex_lock_;
    pthread_mutex_t counter_mutex_lock_;
    vector<sem_t> sem_mail_boxes_;

    bool is_terminate_in_advance_;

    vector<pair<unsigned long, unsigned long>> local_computation_range_index_vec_;
    vector<ElementValueType> global_tasks_vec_;

    ComputationFuncType pair_computation_func_;
    ActionFuncType success_action_func_;
    FailActionFuncType fail_action_func_;

    void InitInnerForLoopComputationTasks();

    void RingCommThreadFunction(unsigned long thread_id);

    void *InvokeRingCommThreadFunction(void *bundle_input_ptr);

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

//Parallelize the inner for loop
template<typename ReduceDataType, typename ComputationFuncType, typename ActionFuncType, typename FailActionFuncType>
void FineGrainedMergeScheduler<ReduceDataType, ComputationFuncType,
        ActionFuncType, FailActionFuncType>::ReduceComputation() {
    std::vector<BundleInput *> input_bundle_vec(thread_count_);
    for (auto &left_element_ptr:*reduce_data_ptr_vector_[right_reduce_data_index_]) {
        left_element_ptr_ = std::move(left_element_ptr);
        InitInnerForLoopComputationTasks();
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
    }
}

template<typename ReduceDataType, typename ComputationFuncType, typename ActionFuncType, typename FailActionFuncType>
unique_ptr<ReduceDataType> FineGrainedMergeScheduler<ReduceDataType, ComputationFuncType,
        ActionFuncType, FailActionFuncType>::Execute() {
    while (reduce_data_ptr_vector_.size() > 1) {
        right_reduce_data_index_ = reduce_data_ptr_vector_.size() - 1;
        ReduceComputation();
        reduce_data_ptr_vector_.erase(reduce_data_ptr_vector_.end() - 1);
    }
    return reduce_data_ptr_vector_[0];
}

template<typename ReduceDataType, typename ComputationFuncType, typename ActionFuncType, typename FailActionFuncType>
void FineGrainedMergeScheduler<ReduceDataType, ComputationFuncType,
        ActionFuncType, FailActionFuncType>::RingCommThreadFunction(
        unsigned long thread_id) {

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

    //Second, initialize the global tasks
    global_tasks_vec_.resize(reduce_data_ptr_vector_[0]->size());
    auto i = 0;
    for (auto &element_ptr:reduce_data_ptr_vector_[0]) {
        global_tasks_vec_[i] = std::move(element_ptr);
        i++;
    }

}


#endif //CODES_YCHE_FIND_GRAINED_MERGE_SCHEDULER_H
