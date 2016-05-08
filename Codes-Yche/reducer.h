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
        unsigned long thread_count_;
        unsigned long idle_count_;
        unsigned long barrier_count_;

        pthread_t *thread_handles;

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
        return nullptr;
    }

    void Reducer::ParallelExecute() {
        //InitLocalData
        InitDataPerThread();
    }


}


#endif //CODES_YCHE_REDUCER_H
