//
// Created by cheyulin on 5/7/16.
//

#ifndef CODES_YCHE_REDUCER_H
#define CODES_YCHE_REDUCER_H

#include "include_header.h"

namespace yche {
    using namespace std;

    class Reducer {
    private:
        unsigned long thread_count_;
        unsigned long idle_count_;
        unsigned long barrier_count_;

        pthread_t *thread_handles;
        vector<sem_t> sem_mail_boxes_;

        vector<bool> is_rec_mail_empty_;

        void LoopCommThreadFunction(unsigned long thread_id);

        static void *InvokeLoopCommThreadFunction(void *bundle_input_ptr);

        void InitDataPerThread();6

    };

}


#endif //CODES_YCHE_REDUCER_H
