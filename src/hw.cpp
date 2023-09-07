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
#include <inttypes.h>

using namespace std;



typedef struct {
    int threadId;
    int numThreads;
} WorkerArgs;

// workerThreadStart --

void* workerThreadStart(void* threadArgs) {
    uint64_t diff, count, num,sum, avg;
    
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
    "mov %1, %%r12 \t\n" //load LEFT to rax
    "mov %2, %%r13 \t\n" //load RIGHT to rbx
    "mov %2, %%r14 \t\n"
    "mov %2, %%rsi \t\n"
    "mov %2, %%rdi \t\n"
    "mov %2, %%r8 \t\n"
    "mov %2, %%r9 \t\n"
    "mov %2, %%r10 \t\n"
    "mov %2, %%r11 \t\n"
    "mov $0x8FFFFFFFFF, %%r15 \t\n" //load loop times
    "rdtsc              \t\n"
    "shl  $32,   %%rdx  \t\n" // get the content of edx
    "or   %%rdx, %%rax  \t\n" // combine the content of edx and eax
    "mov  %%rax, %%rcx  \t\n" // store the time stamp in rcx


    "loop:\n\t"
    "or %%r12, %%r13 \t\n"
    "or %%r12, %%r14 \t\n"
    "or %%r12, %%rsi \t\n"
    "or %%r12, %%rdi \t\n"
    "or %%r12, %%r8 \t\n"
    "or %%r12, %%r9 \t\n"
    "or %%r12, %%r10 \t\n"
    "or %%r12, %%r11 \t\n"

    "or %%r12, %%r13 \t\n"
    "or %%r12, %%r14 \t\n"
    "or %%r12, %%rsi \t\n"
    "or %%r12, %%rdi \t\n"
    "or %%r12, %%r8 \t\n"
    "or %%r12, %%r9 \t\n"
    "or %%r12, %%r10 \t\n"
    "or %%r12, %%r11 \t\n"
    //"jmp loop\n\t"
    "dec  %%r15     \t\n"
    "jnz  loop\t\n"
    "rdtsc            \n\t"
    "shl  $32,   %%rdx  \t\n"
    "or   %%rdx, %%rax  \t\n"
    "sub  %%rcx, %%rax  \t\n"
    "mov  %%rax, %0     \t\n"

    : "=r"(diff)

    : "r" (LEFT), "r" (RIGHT)
  );
    
   printf("\t%" PRIu64"\n",diff);

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

    for (int i=1; i<numThreads; i++){
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

