//
// Created by cheyulin on 4/25/16.
//

#ifndef CODES_YCHE_PARALLELIZER_H
#define CODES_YCHE_PARALLELIZER_H

#include "reducer.h"

namespace yche {
    using namespace std;

    //Define Tags For Template Specialization In parallelizer.h
    struct MergeWithReduce;
    struct MergeSequential;

    template<typename Data>
    struct Task {
        unique_ptr<Data> data_ptr_;

        Task(unique_ptr<Data> data) {
            data_ptr_ = std::move(data);
        }
    };


    template<typename Algorithm, typename MergeType>
    class Parallelizer {
        struct BundleInput {
            Parallelizer *parallelizer_ptr_;
            unsigned long thread_id_;
        };

    private:
        unsigned long thread_count_;
        unsigned long idle_count_;
        unsigned long barrier_count_;

        using BasicData = typename Algorithm::BasicData;
        using MergeData = typename Algorithm::MergeData;
        using ReduceData = typename Algorithm::ReduceData;

        pthread_t *thread_handles;
        unique_ptr<vector<unique_ptr<Task<BasicData>>>> global_computation_task_vec_ptr_;
        vector<vector<unique_ptr<Task<MergeData>>>> merge_task_vecs_;
        vector<pair<unsigned long, unsigned long>> local_computation_range_index_vec_;

        vector<sem_t> sem_mail_boxes_;
        sem_t sem_barrier_;
        sem_t sem_counter_;
        sem_t sem_barrier_counter_;

        pthread_mutex_t merge_mutex_;

        vector<bool> is_rec_mail_empty_;

        bool is_any_merging;

        bool is_end_of_local_computation;

        void LoopCommThreadFunction(unsigned long thread_id);

        static void *InvokeLoopCommThreadFunction(void *bundle_input_ptr);

        void InitTasks();

        void DoLeftMerging();

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
                sem_init(&sem_mail_boxes_[i], 0, 0);
            }


            sem_init(&sem_counter_, 0, 1);
            sem_init(&sem_barrier_, 0, 0);
            sem_init(&sem_barrier_counter_, 0, 1);

            pthread_mutex_init(&merge_mutex_, NULL);

            is_rec_mail_empty_.resize(thread_count_, true);
            is_end_of_local_computation = false;
            is_any_merging = false;
            local_computation_range_index_vec_.resize(thread_count);
            merge_task_vecs_.resize(thread_count_);
            idle_count_ = 0;
            barrier_count_ = 0;
        }


        virtual ~Parallelizer() {
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


    template<typename Algorithm, typename MergeType>
    void Parallelizer<Algorithm, MergeType>::ParallelExecute() {
        struct timespec begin, end;
        double elapsed;
        clock_gettime(CLOCK_MONOTONIC, &begin);

        InitTasks();
        clock_gettime(CLOCK_MONOTONIC, &end);
        elapsed = end.tv_sec - begin.tv_sec;
        elapsed += (end.tv_nsec - begin.tv_nsec) / 1000000000.0;
        cout << "Task Init Cost :" << elapsed << endl;
        cout << "Finish Init" << endl;
        vector<BundleInput *> input_bundle_vec(thread_count_);
        cout << "Thread_count:" << thread_count_ << endl;
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

        DoLeftMerging();


        for (auto i = 0; i < thread_count_; ++i) {
            delete input_bundle_vec[i];
        }
        clock_gettime(CLOCK_MONOTONIC, &end);
        elapsed = end.tv_sec - begin.tv_sec;
        elapsed += (end.tv_nsec - begin.tv_nsec) / 1000000000.0;
        cout << "Elapsed Time " << elapsed << endl;


    }

    template<typename Algorithm, typename MergeType>
    void Parallelizer<Algorithm, MergeType>::InitTasks() {
        auto basic_data_vec_ptr = algorithm_ptr_->InitBasicComputationData();
        global_computation_task_vec_ptr_ = make_unique<vector<unique_ptr<Task<BasicData>>>>();
        for (auto &basic_data_ptr:*basic_data_vec_ptr) {
            global_computation_task_vec_ptr_->push_back(make_unique<Task<BasicData>>(std::move(basic_data_ptr)));
        }
        auto whole_size = global_computation_task_vec_ptr_->size();
        auto avg_size = whole_size / thread_count_;
        for (auto i = 0; i < thread_count_ - 1; i++) {
            local_computation_range_index_vec_[i].first = avg_size * i;
            local_computation_range_index_vec_[i].second = avg_size * (i + 1) - 1;
        }
        local_computation_range_index_vec_[thread_count_ - 1].first = avg_size * (thread_count_ - 1);
        local_computation_range_index_vec_[thread_count_ - 1].second = whole_size - 1;
    }

    template<typename Algorithm, typename MergeType>
    void Parallelizer<Algorithm, MergeType>::LoopCommThreadFunction(unsigned long thread_id) {
        struct timespec begin, end;
        double elapsed;

        unsigned long thread_index = thread_id;
        if (thread_index == 0) {
            clock_gettime(CLOCK_MONOTONIC, &begin);
        }
        auto dst_index = (thread_index + 1) % thread_count_;
        auto src_index = (thread_index - 1 + thread_count_) % thread_count_;
        auto &local_computation_range_pair = local_computation_range_index_vec_[thread_index];
        auto &local_merge_queue = merge_task_vecs_[thread_index];
        while (true) {

            auto local_computation_task_size =
                    local_computation_range_pair.second - local_computation_range_pair.first + 1;
            if (local_computation_task_size == 0) {
                if (idle_count_ == thread_count_ - 1) {
                    is_end_of_local_computation = true;
                    for (auto i = 0; i < thread_count_; ++i) {
                        if (i != dst_index)
                            sem_post(&sem_mail_boxes_[i]);
                    }
                    cout << "Thread Finish!!!  " << thread_index << endl;
                    break;
                }
                else {
                    sem_wait(&sem_counter_);
                    idle_count_++;
                    sem_post(&sem_counter_);

                    is_rec_mail_empty_[dst_index] = false;
                    sem_wait(&sem_mail_boxes_[dst_index]);
                    if (is_end_of_local_computation) {
                        break;
                    }
                    sem_wait(&sem_counter_);
                    idle_count_--;
                    sem_post(&sem_counter_);
                }

            }

            else {
                if (local_computation_task_size > 1) {
                    //Check Flag
                    auto &neighbor_computation_range_pair = local_computation_range_index_vec_[src_index];
                    if (is_rec_mail_empty_[thread_index] == false) {
                        //update neighbor computatin range pair
                        neighbor_computation_range_pair.second = local_computation_range_pair.second;
                        neighbor_computation_range_pair.first =
                                neighbor_computation_range_pair.second - local_computation_task_size / 2 + 1;
                        local_computation_range_pair.second= neighbor_computation_range_pair.first-1;

                        is_rec_mail_empty_[thread_index] = true;

                        sem_post(&sem_mail_boxes_[thread_index]);
                    }
                }
                //Do Local Computation
                auto result = algorithm_ptr_->LocalComputation(
                        std::move((*global_computation_task_vec_ptr_)[local_computation_range_pair.first]->data_ptr_));
                local_computation_range_pair.first++;
                if (is_any_merging) {
                    local_merge_queue.push_back(std::move(make_unique<Task<MergeData>>(std::move(result))));
                }
                else {
                    pthread_mutex_lock(&merge_mutex_);
                    is_any_merging = true;

                    algorithm_ptr_->MergeToGlobal(result);
                    while (local_merge_queue.size() > 0) {
                        auto data = std::move(local_merge_queue.front()->data_ptr_);
                        algorithm_ptr_->MergeToGlobal(data);
                        local_merge_queue.erase(local_merge_queue.begin());
                    }
                    is_any_merging = false;
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


        if (thread_index == 0) {
            clock_gettime(CLOCK_MONOTONIC, &end);
            elapsed = end.tv_sec - begin.tv_sec;
            elapsed += (end.tv_nsec - begin.tv_nsec) / 1000000000.0;
            cout << "Elpased Time In Parallel Computation:" << elapsed << endl;
        }

    }

    template<typename Algorithm, typename MergeType>
    void *Parallelizer<Algorithm, MergeType>::InvokeLoopCommThreadFunction(void *bundle_input_ptr) {
        auto my_bundle_input_ptr = ((BundleInput *) bundle_input_ptr);
        my_bundle_input_ptr->parallelizer_ptr_->LoopCommThreadFunction(my_bundle_input_ptr->thread_id_);
        return NULL;
    }

    template<typename Algorithm, typename MergeType>
    void Parallelizer<Algorithm, MergeType>::DoLeftMerging() {
        vector<unique_ptr<ReduceData>> reduce_data_ptr_vec;
        //Do Left Merging, Current Impl Do not care about the branch cost since it is only called once
        if (std::is_same<MergeType, yche::MergeWithReduce>::value) {
            reduce_data_ptr_vec.push_back(std::move(algorithm_ptr_->overlap_community_vec_));
            for (auto i = 0; i < thread_count_; i++) {
                auto &local_merge_queue = merge_task_vecs_[i];
                while (local_merge_queue.size() > 0) {
                    unique_ptr<MergeData> merge_data_ptr = std::move(local_merge_queue.back()->data_ptr_);
                    local_merge_queue.erase(local_merge_queue.end() - 1);
                    reduce_data_ptr_vec.push_back(
                            std::move(algorithm_ptr_->WrapMergeDataToReduceData(std::move(merge_data_ptr))));
                }
            }

            cout << "Before Reducer" << endl;
            cout << "Reduce Data Size:"<<reduce_data_ptr_vec.size()<<endl;
            Reducer<decltype(reduce_data_ptr_vec), ReduceData, decltype(algorithm_ptr_->CmpReduceData), decltype(algorithm_ptr_->ReduceComputation)> reducer(
                    thread_count_, reduce_data_ptr_vec, algorithm_ptr_->CmpReduceData,
                    algorithm_ptr_->ReduceComputation);
            algorithm_ptr_->overlap_community_vec_ = std::move(reducer.ParallelExecute());

        }
        else if (std::is_same<MergeType, yche::MergeSequential>::value) {
            for (auto i = 0; i < thread_count_; i++) {
                auto &local_merge_queue = merge_task_vecs_[i];
                while (local_merge_queue.size() > 0) {
                    auto data = std::move(local_merge_queue.front()->data_ptr_);
                    algorithm_ptr_->MergeToGlobal(data);
                    local_merge_queue.erase(local_merge_queue.begin());
                }
            }
        }
        else {
            cout << "Specialization Error" << endl;
        }
    }
}


#endif //CODES_YCHE_PARALLELIZER_H
