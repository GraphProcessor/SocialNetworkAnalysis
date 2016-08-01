//
// Created by cheyulin on 8/1/16.
//

#ifndef CODES_YCHE_FIND_GRAINED_MERGE_SCHEDULER_H
#define CODES_YCHE_FIND_GRAINED_MERGE_SCHEDULER_H

#include <memory>
#include <vector>
#include <pthread.h>

using namespace std;

template<typename ReduceDataType, typename CompFunctionType, typename PredicateType>
class FineGrainedMergeScheduler {
public:
    unique_ptr<ReduceDataType> Execute();

private:
    struct BundleInput {
        FineGrainedMergeScheduler *fine_grained_merge_scheduler_ptr_;
        unsigned long thread_id_;

    };

    vector<unique_ptr<ReduceDataType>> reduce_data_ptr_vector_;

    unsigned long right_reduce_data_index_;

    unsigned long thread_count_;

    pthread_t *thread_handles_;

    CompFunctionType pair_computation_func_;
    PredicateType pair_computation_terminal_predicate_func_;

    void RingCommTaskStealAndRequestThreadFunction(unsigned long thread_id);

    void *InvokeRingCommThreadFunction(void *bundle_input_ptr);

    void ReduceComputation();
};


template<typename ReduceDataType, typename CompFuncType, typename PredicateType>
void *FineGrainedMergeScheduler<ReduceDataType, CompFuncType, PredicateType>::InvokeRingCommThreadFunction(
        void *bundle_input_ptr) {
    auto my_bundle_input_ptr = ((BundleInput *) bundle_input_ptr);
    my_bundle_input_ptr->fine_grained_merge_scheduler_ptr_->RingCommTaskStealAndRequestThreadFunction(
            my_bundle_input_ptr->thread_id_);
    return NULL;
}

//Parallelize the inner for loop
template<typename ReduceDataType, typename CompFuncType, typename PredicateType>
void FineGrainedMergeScheduler<ReduceDataType, CompFuncType, PredicateType>::ReduceComputation() {
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
}

template<typename ReduceDataType, typename CompFuncType, typename PredicateType>
unique_ptr<ReduceDataType> FineGrainedMergeScheduler<ReduceDataType, CompFuncType, PredicateType>::Execute() {
    while (reduce_data_ptr_vector_.size() > 1) {
        right_reduce_data_index_ = reduce_data_ptr_vector_.size() - 1;
        ReduceComputation();
        reduce_data_ptr_vector_.erase(reduce_data_ptr_vector_.end() - 1);
    }
    return reduce_data_ptr_vector_[0];
}

template<typename ReduceDataType, typename CompFuncType, typename PredicateType>
void FineGrainedMergeScheduler<ReduceDataType, CompFuncType, PredicateType>::RingCommTaskStealAndRequestThreadFunction(
        unsigned long thread_id) {
    for (auto &left_element_ptr:*reduce_data_ptr_vector_[0]) {
        for (auto &right_element_ptr:*reduce_data_ptr_vector_[right_reduce_data_index_]) {
            pair_computation_func_(left_element_ptr, right_element_ptr);
        }
    }
    //Final Result Store in left_reduce_data, right_reduce_data is going to be erased

}


#endif //CODES_YCHE_FIND_GRAINED_MERGE_SCHEDULER_H
