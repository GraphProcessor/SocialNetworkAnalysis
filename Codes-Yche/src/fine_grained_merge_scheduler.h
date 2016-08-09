//
// Created by cheyulin on 8/8/16.
//

#ifndef CODES_YCHE_FINE_GRAINED_MERGE_SCHEDULER_H
#define CODES_YCHE_FINE_GRAINED_MERGE_SCHEDULER_H

#include <boost/range.hpp>
#include <memory>
#include <vector>
#include "semaphore.h"
#include <iostream>
#include <pthread.h>


#include "configuration.h"
#include "thread_pool_breakable.h"

using namespace std;
using namespace yche;
namespace yche {
    template<typename ReduceDataType, typename ComputationFuncType, typename ActionFuncType, typename FailActionFuncType>
    class FineGrainedMergeScheduler {
    public:
        FineGrainedMergeScheduler(unsigned long thread_count,
                                  vector<unique_ptr<ReduceDataType>> &&reduce_data_ptr_vector,
                                  ComputationFuncType pair_computation_func, ActionFuncType success_action_func,
                                  FailActionFuncType fail_action_func)
                : thread_count_(thread_count), pair_computation_func_(pair_computation_func),
                  success_action_func_(success_action_func), fail_action_func_(fail_action_func),
                  thread_pool_(thread_count) {
            reduce_data_ptr_vector_ = std::move(reduce_data_ptr_vector);
        }

        unique_ptr<ReduceDataType> Execute() {
            while (reduce_data_ptr_vector_.size() > 1) {
                cout << "Left Reduce Size:" << reduce_data_ptr_vector_.size() << endl;
                right_reduce_data_index_ = reduce_data_ptr_vector_.size() - 1;
                ReduceComputation();
                reduce_data_ptr_vector_.erase(reduce_data_ptr_vector_.end() - 1);
            }
            return std::move(reduce_data_ptr_vector_[0]);
        }

        virtual  ~FineGrainedMergeScheduler() {
        }

    private:
        ThreadPoolBreakable thread_pool_;
        unsigned long thread_count_;

        vector<unique_ptr<ReduceDataType>> reduce_data_ptr_vector_;
        unsigned long right_reduce_data_index_;
        using ElementValueType = typename boost::range_value<ReduceDataType>::type;
        ElementValueType left_element_ptr_;

        ComputationFuncType pair_computation_func_;
        ActionFuncType success_action_func_;
        FailActionFuncType fail_action_func_;

        void ReduceComputation();
    };

    //Parallel the inner for loop
    template<typename ReduceDataType, typename ComputationFuncType, typename ActionFuncType, typename FailActionFuncType>
    void FineGrainedMergeScheduler<ReduceDataType, ComputationFuncType,
            ActionFuncType, FailActionFuncType>::ReduceComputation() {
        int round_num = 0;
        //Here make use of thread pool
        for (auto &left_element_ptr:*reduce_data_ptr_vector_[right_reduce_data_index_]) {
            left_element_ptr_ = std::move(left_element_ptr);
            bool is_terminate_in_advance = false;
            for (auto &right_element_ptr:*reduce_data_ptr_vector_[0]) {
                std::function<BreakWithCallBackRetType(void)> task_function = [&right_element_ptr, this]() {
                    if (this->pair_computation_func_(this->left_element_ptr_, right_element_ptr)) {
                        return BreakWithCallBackRetType(true, std::bind(this->success_action_func_,
                                                                        std::ref(this->left_element_ptr_),
                                                                        std::ref(right_element_ptr)));
                    }
                    else
                        return BreakWithCallBackRetType();
                };
                thread_pool_.AddTask(task_function);
            }
            thread_pool_.WaitForBreakOrTerminate(is_terminate_in_advance);
            if (!is_terminate_in_advance) {
                fail_action_func_(left_element_ptr_, reduce_data_ptr_vector_[0]);
            }
            cout << "Round:" << ++round_num << endl;
        }
    }
}

#endif //CODES_YCHE_FINE_GRAINED_MERGE_SCHEDULER_H
