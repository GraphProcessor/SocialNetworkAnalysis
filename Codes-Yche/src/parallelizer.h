//
// Created by cheyulin on 4/25/16.
//

#ifndef CODES_YCHE_PARALLELIZER_H
#define CODES_YCHE_PARALLELIZER_H

#include "reducer.h"

namespace yche {
    using namespace std;

    template<typename Algorithm>
    class Parallelizer {
        struct BundleInput {
            Parallelizer *parallelizer_ptr_;
            unsigned long thread_id_;
        };

    private:
        unsigned long thread_count_;
        unsigned long idle_count_;

        using BasicData = typename Algorithm::BasicData;
        using MergeData = typename Algorithm::MergeData;
        using ReduceData = typename Algorithm::ReduceData;


        unique_ptr<vector<unique_ptr<BasicData>>> global_computation_task_vec_ptr_;
        vector<pair<unsigned long, unsigned long>> local_computation_range_index_vec_;

        vector<vector<unique_ptr<MergeData>>> merge_task_vecs_;
        vector<vector<unique_ptr<ReduceData>>> reduce_task_vectors_;

        pthread_t *thread_handles;
        pthread_mutex_t counter_mutex_;
        pthread_mutex_t merge_mutex_;
        pthread_barrier_t timestamp_barrier;

        vector<bool> is_rec_mail_empty_;

        bool is_end_of_local_computation;

        void RingCommThreadFunction(unsigned long thread_id);

        static void *InvokeRingCommThreadFunction(void *bundle_input_ptr);

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

            pthread_mutex_init(&counter_mutex_, NULL);
            pthread_mutex_init(&merge_mutex_, NULL);
            pthread_barrier_init(&timestamp_barrier, NULL, thread_count);

            is_rec_mail_empty_.resize(thread_count_, true);
            is_end_of_local_computation = false;
            local_computation_range_index_vec_.resize(thread_count);
            merge_task_vecs_.resize(thread_count_);
            reduce_task_vectors_.resize(thread_count_);
            idle_count_ = 0;
        }


        virtual ~Parallelizer() {
            pthread_mutex_destroy(&counter_mutex_);
            pthread_mutex_destroy(&merge_mutex_);
            pthread_barrier_destroy(&timestamp_barrier);

            delete[]thread_handles;
        }
    };


    template<typename Algorithm>
    void Parallelizer<Algorithm>::ParallelExecute() {
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
            pthread_create(&thread_handles[thread_id], NULL, this->InvokeRingCommThreadFunction,
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

    template<typename Algorithm>
    void Parallelizer<Algorithm>::InitTasks() {
        auto basic_data_vec_ptr = algorithm_ptr_->InitBasicComputationData();
        global_computation_task_vec_ptr_ = make_unique<vector<unique_ptr<BasicData>>>();
        for (auto &basic_data_ptr:*basic_data_vec_ptr) {
            global_computation_task_vec_ptr_->push_back(std::move(basic_data_ptr));
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

    template<typename Algorithm>
    void Parallelizer<Algorithm>::RingCommThreadFunction(unsigned long thread_id) {
        struct timespec begin, end;
        double elapsed;

        unsigned long thread_index = thread_id;
        if (thread_index == 0) {
            clock_gettime(CLOCK_MONOTONIC, &begin);
        }
        auto dst_index = (thread_index + 1) % thread_count_;
        auto src_index = (thread_index - 1 + thread_count_) % thread_count_;
        auto &local_computation_range_pair = local_computation_range_index_vec_[thread_index];
        auto &local_reduce_queue = reduce_task_vectors_[thread_index];

        while (!is_end_of_local_computation) {
            auto local_computation_task_size =
                    local_computation_range_pair.second - local_computation_range_pair.first + 1;
            if (local_computation_task_size == 0) {
                if (idle_count_ == thread_count_ - 1) {
                    is_end_of_local_computation = true;
                    cout << "Thread Finish!!!  " << thread_index << endl;
                    break;
                }
                else {
                    pthread_mutex_lock(&counter_mutex_);
                    idle_count_++;
                    pthread_mutex_unlock(&counter_mutex_);

                    is_rec_mail_empty_[dst_index] = false;

                    while (!is_rec_mail_empty_[dst_index]) {
                        if (local_reduce_queue.size() > 1) {
                            local_reduce_queue.front() = algorithm_ptr_->ReduceComputation(local_reduce_queue.front(),
                                                                                           local_reduce_queue.back());
                            local_reduce_queue.erase(local_reduce_queue.end() - 1);
                        }
                        if (is_end_of_local_computation) {
                            cout << "Finish Local Computation" << thread_index << endl;
                            break;
                        }
                    }
                    pthread_mutex_lock(&counter_mutex_);
                    idle_count_--;
                    pthread_mutex_unlock(&counter_mutex_);
                }
            }

            else {
                if (local_computation_task_size > 1) {
                    //Check Flag
                    auto &neighbor_computation_range_pair = local_computation_range_index_vec_[src_index];
                    if (is_rec_mail_empty_[thread_index] == false) {
                        //update neighbor computation range pair
                        neighbor_computation_range_pair.second = local_computation_range_pair.second;
                        neighbor_computation_range_pair.first =
                                neighbor_computation_range_pair.second - local_computation_task_size / 2 + 1;
                        local_computation_range_pair.second = neighbor_computation_range_pair.first - 1;

                        is_rec_mail_empty_[thread_index] = true;
                    }
                }
                //Do Local Computation
                auto result = algorithm_ptr_->LocalComputation(
                        std::move((*global_computation_task_vec_ptr_)[local_computation_range_pair.first]));
                local_computation_range_pair.first++;

                //Directly Put into Reduce Task Vector
                local_reduce_queue.push_back(std::move(algorithm_ptr_->WrapMergeDataToReduceData(result)));
            }
        }

        //Barrier
        pthread_barrier_wait(&timestamp_barrier);

        if (thread_index == 0) {
            clock_gettime(CLOCK_MONOTONIC, &end);
            elapsed = end.tv_sec - begin.tv_sec;
            elapsed += (end.tv_nsec - begin.tv_nsec) / 1000000000.0;
            cout << "Elapsed Time In Parallel Computation:" << elapsed << endl;
        }

    }

    template<typename Algorithm>
    void *Parallelizer<Algorithm>::InvokeRingCommThreadFunction(void *bundle_input_ptr) {
        auto my_bundle_input_ptr = ((BundleInput *) bundle_input_ptr);
        my_bundle_input_ptr->parallelizer_ptr_->RingCommThreadFunction(my_bundle_input_ptr->thread_id_);
        return NULL;
    }

    template<typename Algorithm>
    void Parallelizer<Algorithm>::DoLeftMerging() {
        vector<unique_ptr<ReduceData>> reduce_data_ptr_vec;
        //Do Left Merging, Current Impl Do not care about the branch cost since it is only called once
        for (auto i = 0; i < thread_count_; i++) {
            auto &local_reduce_queue = reduce_task_vectors_[i];
            while (local_reduce_queue.size() > 0) {
                unique_ptr<ReduceData> reduce_data_ptr = std::move(local_reduce_queue.back());
                local_reduce_queue.erase(local_reduce_queue.end() - 1);
                reduce_data_ptr_vec.push_back(std::move(reduce_data_ptr));
            }
        }

        cout << "Before Reducer" << endl;
        cout << "Reduce Data Size:" << reduce_data_ptr_vec.size() << endl;
        Reducer<decltype(reduce_data_ptr_vec), ReduceData, decltype(algorithm_ptr_->CmpReduceData), decltype(algorithm_ptr_->ReduceComputation)> reducer(
                thread_count_, reduce_data_ptr_vec, algorithm_ptr_->CmpReduceData,
                algorithm_ptr_->ReduceComputation);
        algorithm_ptr_->overlap_community_vec_ = std::move(reducer.ParallelExecute());
    }
}


#endif //CODES_YCHE_PARALLELIZER_H
