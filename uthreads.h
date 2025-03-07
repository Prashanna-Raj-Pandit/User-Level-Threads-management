#ifndef UTHREADS_H  
#define UTHREADS_H

#include <ucontext.h>
#include <stdlib.h>

#define STACK_SIZE 1024 * 8  // 8 KB stack per thread
#define NUM_THREADS 4

int queue_size = 0;


typedef struct {
    ucontext_t context;
    int id;
    int tickets;  
    int iterations;  
} pp_threads;      // means prashanna pandit threads

pp_threads* current_thread = NULL;
pp_threads* ready_queue[NUM_THREADS];


pp_threads* lotteryScheduler();
pp_threads* create_thread(void (*func)(), int id, int tickets);


#endif  