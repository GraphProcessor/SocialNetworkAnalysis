//
// Created by cheyulin on 5/7/16.
//

#ifndef CODES_YCHE_REDUCER_H
#define CODES_YCHE_REDUCER_H

#include "include_header.h"

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
        unsigned long idle_count_;
        unsigned long barrier_count_;

        pthread_t *thread_handles;

        vector<unique_ptr<Data>> global_data_vec_;
        vector<unique_ptr<Data>> local_data_vec_;
        vector<sem_t> sem_mail_boxes_;
        vector<bool> is_rec_mail_empty_;

        DataCmpFunction data_cmp_function_;
        ComputationFunction compute_fucntion_;

        void LoopCommThreadFunction(unsigned long thread_id);

        static void *InvokeLoopCommThreadFunction(void *bundle_input_ptr);

        void InitDataPerThread();

    public:

        void ParallelExecute();

        Reducer(unsigned long thread_count_, DataCollection reduce_data, DataCmpFunction data_cmp_function_,
                ComputationFunction compute_fucntion_)
                : thread_count_(thread_count_), data_cmp_function_(data_cmp_function_),
                  compute_fucntion_(compute_fucntion_) {
            thread_handles = new pthread_t[thread_count_];
            sem_mail_boxes_.resize(thread_count_);
            for (auto i = 0; i < thread_count_; ++i) {
                sem_init(&sem_mail_boxes_[i], NULL, 0);
            }

        }

        virtual ~Reducer() {
            for (auto i = 0; i < thread_count_; ++i) {
                sem_destroy(&sem_mail_boxes_[i]);
            }
            delete[]thread_handles;
        }
    };


    void Reducer::InitDataPerThread() {

    }

    void Reducer::LoopCommThreadFunction(unsigned long thread_id) {

    }

    void *Reducer::InvokeLoopCommThreadFunction(void *bundle_input_ptr) {
        auto my_bundle_input_ptr = ((BundleInput *) bundle_input_ptr);
        my_bundle_input_ptr->reducer_ptr_->LoopCommThreadFunction(my_bundle_input_ptr->thread_id_);
    }

    void Reducer::ParallelExecute() {
        //InitLocalData
        InitDataPerThread();
        vector<BundleInput *> input_bundle_vec(thread_count_);
        for (auto thread_id = 0; thread_id < thread_count_; thread_id++) {
            input_bundle_vec[thread_id] = new BundleInput();
            input_bundle_vec[thread_id]->parallelizer_ptr_ = this;
            input_bundle_vec[thread_id]->thread_id_ = thread_id;
            pthread_create(&thread_handles[thread_id], NULL, this->InvokeLoopCommThreadFunction,
                           (void *) input_bundle_vec[thread_id]);
        }

        for (auto thread_id = 0; thread_id < thread_count_; thread_id++) {
            pthread_join(thread_handles[thread_id], NULL);
        }

        //Delete After All Execution-Flow Join
        for (auto i = 0; i < thread_count_; ++i) {
            delete input_bundle_vec[i];
        }

    }


}


#endif //CODES_YCHE_REDUCER_H
