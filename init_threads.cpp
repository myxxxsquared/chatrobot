
#include "chatrobot.hpp"

pthread_t thread_dialog, thread_recognize, thread_generate;

void init_threads()
{
    if(0 != pthread_create(&thread_dialog, NULL, &do_dialog, NULL))
        throw "pthread_create(thread_dialog) Failed.";
    if(0 != pthread_create(&thread_recognize, NULL, &do_recognize, NULL))
        throw "pthread_create(thread_recognize) Failed.";
    if(0 != pthread_create(&thread_generate, NULL, &do_generate, NULL))
        throw "pthread_create(thread_generate) Failed.";
}

void init_threads_offline()
{
//    if(0 != pthread_create(&thread_dialog, NULL, &do_dialog, NULL))
//        throw "pthread_create(thread_dialog) Failed.";
    if(0 != pthread_create(&thread_recognize, NULL, &do_recognize_offline, NULL))
        throw "pthread_create(thread_recognize) Failed.";
//    if(0 != pthread_create(&thread_generate, NULL, &do_generate, NULL))
//        throw "pthread_create(thread_generate) Failed.";
}
