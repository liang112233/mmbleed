/*
 *
 * g++ -m64  -pthread pthread_arr.cpp -o pthread_arr

 */
#include <stdio.h>
#include <pthread.h>
#include <stdint.h>
#include <errno.h>
#include <err.h>
#include <stdlib.h>
#include <mutex>
#include <thread>
#include<iostream>
using namespace std;



typedef struct {
    int threadId;
    int numThreads;
} WorkerArgs;

// workerThreadStart --

void* workerThreadStart(void* threadArgs) {
    
    std::mutex iomutex;

    WorkerArgs* args = static_cast<WorkerArgs*>(threadArgs);

    int threadID = (args->threadId);
    // to test thread affinity
    /*
    while(1){
        std::lock_guard<std::mutex> iolock(iomutex);

        cout<< "Thread ID " << threadID << " is on CPU " << sched_getcpu() <<"\n";

        std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    }*/

    //cout<< "Thread ID " << threadID << " is on CPU " << sched_getcpu() <<"\n";

    uint64_t value,LEFT, RIGHT;
    //hw weight
    value = 0xFFFFFFFFFFFFFFFF;
    LEFT = value;
    RIGHT = value;
    asm volatile(
    "mov %0, %%rax \t\n" //load LEFT to rax
    "mov %1, %%rcx \t\n" //load RIGHT to rbx
    "mov %1, %%rdx \t\n"
    "mov %1, %%rsi \t\n"
    "mov %1, %%rdi \t\n"
    "mov %1, %%r8 \t\n"
    "mov %1, %%r9 \t\n"
    "mov %1, %%r10 \t\n"
    "mov %1, %%r11 \t\n"

    "loop:\n\t"
    "or %%rax, %%rcx \t\n"
    "or %%rax, %%rdx \t\n"
    "or %%rax, %%rsi \t\n"
    "or %%rax, %%rdi \t\n"
    "or %%rax, %%r8 \t\n"
    "or %%rax, %%r9 \t\n"
    "or %%rax, %%r10 \t\n"
    "or %%rax, %%r11 \t\n"
    "jmp loop\n\t"
    :// no output  
    : "r" (LEFT), "r" (RIGHT)
  );
    return NULL;
}


// Multi-threaded implementation of mandelbrot set image generation.
// Multi-threading performed via pthreads.
void arrayThread(int numThreads)
{

    pthread_t workers[numThreads];
    WorkerArgs args[numThreads];
    
    for (int i=0; i<numThreads; i++) {
    // threads args
    args[i].threadId = i;
    args[i].numThreads = numThreads;

    }
    // printf("num of threads %d\n", numThreads);
    // Fire up the worker threads.  Note that numThreads-1 pthreads
    // are created and the main app thread is used as a worker as
    // well.

    for (int i=0; i<numThreads; i++){
        pthread_create(&workers[i], NULL, workerThreadStart, &args[i]);

        //create cpu set
   
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(i,&cpuset);
        //set affinity
        int s = pthread_setaffinity_np(workers[i], sizeof(cpu_set_t), &cpuset);

        if(s != 0)  
            cerr <<"Error calling pthread_setaffinity_np: " << s << "\n";

    }

    workerThreadStart(&args[0]);

    // wait for worker threads to complete
    for (int i=1; i<numThreads; i++)
        pthread_join(workers[i], NULL);
}

int main()
{
    int numThreads = 8;
    arrayThread(numThreads);
    return 0;
}

