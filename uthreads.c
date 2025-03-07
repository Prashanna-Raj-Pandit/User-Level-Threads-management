/*
Author Name: 
    - Prashanna Raj Pandit (800817018) SIUE
--------------------------------------------------------------------------------------------------------
Program Information: User-Level Threads Implementation by RR and Lottery Schedulers
---------------------------------------------------------------------------------------------------------
Submitted on: Mar 06, 2025
---------------------------------------------------------------------------------------------------------
Deadline: Mar 07, 2025
---------------------------------------------------------------------------------------------------------
Program Description:
    - This program is an implementation of User-Level Threads using Round-Robin and Lottery Schedulers.
    - The program creates 2 threads and runs them using the Round-Robin Scheduler. (Preemption)
    - The program creates 4 threads and runs them using the Round-Robin Scheduler.(2 threads yield, 2 threads preempt)
    - The program creates 2 threads and runs them using the Lottery Scheduler.(Preemption)
    - The program also creates 2 threads that yield the CPU.
    - The program also stops execution after a specified time.
---------------------------------------------------------------------------------------------------------
Compilation Instructions:
To compile: make
To Run: ./uthreads
---------------------------------------------------------------------------------------------------------
*/

#include <signal.h>
#include <unistd.h>
#include <ucontext.h>
#include <string.h>
#include <time.h>
#include "uthreads.h"  
#include <sys/time.h>
#include<stdio.h>
#include<stdlib.h>

#define TIME_SLICE 10000  // 100 ms
int scheduler_choice;
int stopTimer=0;


pp_threads* getNextThread_by_LS(){
    pp_threads* next_thread = lotteryScheduler();
    if (next_thread == NULL) {
        printf("No threads to schedule!\n");
        exit(0);
    }
    printf("Switching to Thread %d (Tickets: %d)\n", next_thread->id, next_thread->tickets);
    return next_thread;
}
pp_threads* getNextThread_by_RR(){
    pp_threads* next_thread = ready_queue[0];
    if (next_thread == NULL) {
        printf("No threads to schedule!\n");
        exit(0);
    }
    for (int i = 1; i < queue_size; i++) {
        ready_queue[i - 1] = ready_queue[i];
    }
    queue_size--;
    printf("Switching to Thread %d\n", next_thread->id);
    return next_thread;
}

pp_threads* lotteryScheduler() {
    int total_tickets = 0;
    for (int i = 0; i < queue_size; i++) {
        total_tickets += ready_queue[i]->tickets;
    }
    if (total_tickets == 0) {
        return NULL;  
    }
    int ticket_selected = rand() % total_tickets;  // Randomly select a winning ticket
    int ticket_collection = 0;
    for (int i = 0; i < queue_size; i++) {
        ticket_collection += ready_queue[i]->tickets;
        if (ticket_collection>ticket_selected) {
            pp_threads* selected_thread = ready_queue[i];
            for (int j = i; j < queue_size - 1; j++) {
                ready_queue[j] = ready_queue[j + 1];
            }
            queue_size--;
            return selected_thread;
        }
    }
    return NULL;  
}

void stopExecution(int signalNumber) {
    printf("CPU time limit (%d sec) reached! Stopping execution.\n",stopTimer);
    exit(0);
}

void schedulerHandler(int signalNumber) {
    if (current_thread != NULL) {
        ready_queue[queue_size++] = current_thread;
    }
    pp_threads* previous_thread = current_thread;

    if (scheduler_choice==1 || scheduler_choice==2){
        current_thread=getNextThread_by_RR();
    }else if(scheduler_choice==3){
    current_thread = getNextThread_by_LS();}
    else{
        printf("Invalid Scheduler.\n");
    }

    if (previous_thread) {
        swapcontext(&previous_thread->context, &current_thread->context);
    } else {
        setcontext(&current_thread->context);  // for first thread
    }
}

// Timer setup function
void setTimer() {
    signal(SIGVTALRM, schedulerHandler); // For preemption every 100ms
    signal(SIGPROF, stopExecution);  // To stop execution after x CPU seconds, specified by the user

    struct itimerval virtual_timer;
    virtual_timer.it_value.tv_sec = 0;
    virtual_timer.it_value.tv_usec = TIME_SLICE;  
    virtual_timer.it_interval = virtual_timer.it_value;
    setitimer(ITIMER_VIRTUAL, &virtual_timer, NULL);  

    
    struct itimerval stop_timer;
    stop_timer.it_value.tv_sec = stopTimer;
    stop_timer.it_value.tv_usec = 0;
    stop_timer.it_interval.tv_sec = 0;  
    stop_timer.it_interval.tv_usec = 0;
    setitimer(ITIMER_PROF, &stop_timer, NULL);  
}

void thread1() {
    while (1) {
        current_thread->iterations++;
        printf("Thread 1: Iteration %d\n", current_thread->iterations);
        for (int i = 0; i < 1000000; i++);
    }
}

void thread2() {
    while (1) {
        current_thread->iterations++;
        printf("Thread 2: Iteration %d\n", current_thread->iterations);
        for (int i = 0; i < 1000000; i++);
    }
}

// Threads functions for yielding threads. i.e y_threads  
void y_thread3() {
    while (1) {
        current_thread->iterations++;
        printf("Thread 3: Iteration %d\n", current_thread->iterations);
        printf("Thread Yielding\n");
        schedulerHandler(0); // Simulate yielding
    }
}

void y_thread4() {
    while (1) {
        current_thread->iterations++;
        printf("Thread 4: Iteration %d\n", current_thread->iterations);
        printf("Thread Yielding\n");
        schedulerHandler(0);
    }
}

pp_threads* create_thread(void (*func)(), int id, int tickets) {
    pp_threads* new_thread = (pp_threads*)malloc(sizeof(pp_threads));
    getcontext(&new_thread->context);
    new_thread->id = id;
    new_thread->tickets = tickets;
    new_thread->iterations = 0;
    new_thread->context.uc_stack.ss_sp = malloc(STACK_SIZE);  // Allocate stack memory
    new_thread->context.uc_stack.ss_size = STACK_SIZE;
    new_thread->context.uc_link = &current_thread->context;
    makecontext(&new_thread->context, func, 0);
    return new_thread;
}

void create_yielding_threads(){
    pp_threads* yielding_thread1=create_thread(y_thread3,3,10);
    pp_threads* yielding_thread2=create_thread(y_thread4,4,10);

    ready_queue[queue_size++]=yielding_thread1;
    ready_queue[queue_size++]=yielding_thread2;
}
void initializeThreads_and_ReadyQueue(){
    pp_threads* prashanna = create_thread(thread1, 1, 10);  
    pp_threads* pandit = create_thread(thread2, 2, 20);  
    
    ready_queue[queue_size++] = prashanna;
    ready_queue[queue_size++] = pandit;

    if (scheduler_choice==1){
        create_yielding_threads();
        printf("4 Threads Created.\n");
    }
}

int main() {
    do{
    printf("Choose your scheduler:\n"
            "1. Round-Robin Scheduler:\n"
            "   - Runs 4 threads.\n"
            "   - 2 threads yield, 2 threads preempt.(press 1)\n"
            "2. Round-Robin Scheduler (Simplified):\n"
            "   - Runs 2 threads by preemption. (Press 2)\n"
            "3. Lottery Scheduler:(Verifies Waldspurger's experiments 5.1)\n"
            "   - Runs 2 threads by preemption. (Press 3)\n");
    scanf("%d",&scheduler_choice);
    if(scheduler_choice!=1 && scheduler_choice!=2 && scheduler_choice!=3){
        printf("Invalid scheduler choice.\n");
    }
    }while(scheduler_choice!=1 && scheduler_choice!=2 && scheduler_choice!=3);

    printf("Enter the stopping time. (in seconds):");
    scanf("%d",&stopTimer);
    initializeThreads_and_ReadyQueue();
    setTimer();
    schedulerHandler(0);
    return 0;
}
