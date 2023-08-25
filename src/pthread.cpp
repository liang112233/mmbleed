/*
 *
 * * g++ -m64  -pthread pthread_arr.cpp -o pthread_arr

 */
#include <thread>
#include <chrono>
#include <stdio.h>
#include <pthread.h>
#include <stdint.h>
#include <mutex>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>

#include <iostream>
using namespace std;
#define MEASURE_ROUND 500

/* Package RAPL Domain */
#define MSR_PKG_RAPL_POWER_LIMIT  0x610
#define MSR_PKG_ENERGY_STATUS   0x611
#define MSR_PKG_PERF_STATUS   0x613
#define MSR_PKG_POWER_INFO    0x614

/* PP0 RAPL Domain */
#define MSR_PP0_POWER_LIMIT   0x638
#define MSR_PP0_ENERGY_STATUS   0x639
#define MSR_PP0_POLICY      0x63A
#define MSR_PP0_PERF_STATUS   0x63B

/* PP1 RAPL Domain, may reflect to uncore devices */
#define MSR_PP1_POWER_LIMIT   0x640
#define MSR_PP1_ENERGY_STATUS   0x641
#define MSR_PP1_POLICY      0x642

/* DRAM RAPL Domain */
#define MSR_DRAM_POWER_LIMIT    0x618
#define MSR_DRAM_ENERGY_STATUS    0x619
#define MSR_DRAM_PERF_STATUS    0x61B
#define MSR_DRAM_POWER_INFO   0x61C

#define MAX_CPUS  1024
#define MAX_PACKAGES  16

#define MSR_RAPL_POWER_UNIT   0x606

static int open_msr(int core) {

  char msr_filename[BUFSIZ];
  int fd;

  sprintf(msr_filename, "/dev/cpu/%d/msr", core);
  fd = open(msr_filename, O_RDONLY);
  if ( fd < 0 ) {
    if ( errno == ENXIO ) {
      fprintf(stderr, "rdmsr: No CPU %d\n", core);
      exit(2);
    } else if ( errno == EIO ) {
      fprintf(stderr, "rdmsr: CPU %d doesn't support MSRs\n",
          core);
      exit(3);
    } else {
      perror("rdmsr:open");
      fprintf(stderr,"Trying to open %s\n",msr_filename);
      exit(127);
    }
  }

  return fd;
}

static long long read_msr(int fd, int which) {

  uint64_t data;

  if ( pread(fd, &data, sizeof data, which) != sizeof data ) {
    perror("rdmsr:pread");
    exit(127);
  }

  return (long long)data;
}



typedef struct {
    int threadId;
    int numThreads;
} WorkerArgs;

// workerThreadStart --

void* workerThreadStart(void* threadArgs) {
    
    int fd;
    long long result;
    double power_units,time_units;
    double cpu_energy_units[MAX_PACKAGES],dram_energy_units[MAX_PACKAGES];
    double package_before[MAX_PACKAGES],package_after[MAX_PACKAGES];
    double pp0_before[MAX_PACKAGES],pp0_after[MAX_PACKAGES];
    double pp1_before[MAX_PACKAGES],pp1_after[MAX_PACKAGES];
    double dram_before[MAX_PACKAGES],dram_after[MAX_PACKAGES];
    int dram_avail=0,pp0_avail=0,pp1_avail=0;
    pp0_avail=1;
    pp1_avail=1;
    dram_avail=1;
    static int package_map[MAX_PACKAGES];


    WorkerArgs* args = static_cast<WorkerArgs*>(threadArgs);
    std::mutex iomutex;

    int data[MEASURE_ROUND];
    int threadID = (args->threadId);

    double package_pow[MEASURE_ROUND];
    double pp0_pow[MEASURE_ROUND];
    double pp1_pow[MEASURE_ROUND];
    double dram_pow[MEASURE_ROUND];
    
    double package_sum = 0;
    double pp0_sum = 0;
    double pp1_sum = 0;
    double dram_sum = 0;
    double pkg_avg;
    double pp0_avg;
    double pp1_avg;
    double dram_avg;

    /*
     * while(1){
        std::lock_guard<std::mutex> iolock(iomutex);

        cout<< "Thread ID " << threadID << " is on CPU " << sched_getcpu() <<"\n";
    
        std::this_thread::sleep_for(std::chrono::milliseconds(900));

    }*/
    uint32_t diff, avg, n  ;
    uint64_t count, num,sum;
    count = 15;
    sum = 0;
    num = 0xFFFFFFFF0000;

    for (int n=0; n< MEASURE_ROUND; n++){
        
        fd=open_msr(0);

        /* Calculate the units used */
        result=read_msr(fd,MSR_RAPL_POWER_UNIT);
        power_units=pow(0.5,(double)(result&0xf));
        cpu_energy_units[0]=pow(0.5,(double)((result>>8)&0x1f));
        time_units=pow(0.5,(double)((result>>16)&0xf));
        dram_energy_units[0]=cpu_energy_units[0];


        /* Package Energy */
        result=read_msr(fd,MSR_PKG_ENERGY_STATUS);
        package_before[0]=(double)result*cpu_energy_units[0];
        result=read_msr(fd,MSR_PP0_ENERGY_STATUS);
        pp0_before[0]=(double)result*cpu_energy_units[0];
        result=read_msr(fd,MSR_PP1_ENERGY_STATUS);
        pp1_before[0]=(double)result*cpu_energy_units[0];
        result=read_msr(fd,MSR_DRAM_ENERGY_STATUS);
        dram_before[0]=(double)result*dram_energy_units[0];

        
        asm volatile(
        "mov %1, %%r13 \t\n" //load count to rax
        "mov %2, %%r14 \t\n" //load num to rbx
        "mov $200000000, %%r15 \t\n" //load loop times
        "nop           \t\n"
        "nop           \t\n"
        "nop           \t\n"
        "nop           \t\n"
        "nop           \t\n"
        "nop           \t\n"
        "nop           \t\n"
        "nop           \t\n"
        "nop           \t\n"
        "nop           \t\n"

        "rdtsc              \t\n"
        "movl %%eax, %%ecx  \t\n"
        "loop_start:\n\t"
        "shlx %%r13, %%r14, %%rdx \t\n"
        "shlx %%r13, %%r14, %%rsi \t\n"
        "shrx %%r13, %%r14, %%rdi \t\n"
        "shrx %%r13, %%r14, %%r8 \t\n"
        "shlx %%r13, %%r14, %%r9 \t\n"
        "shlx %%r13, %%r14, %%r10 \t\n"
        "shrx %%r13, %%r14, %%r11 \t\n"
        "shrx %%r13, %%r14, %%r12 \t\n"
        //"jmp loop_start\n\t"
        "dec  %%r15     \t\n"
        "jnz  loop_start\t\n"
        "rdtsc            \n\t"
        "subl %%ecx, %%eax  \n\t"
        "movl %%eax, %0     \n\t"

        : "=r"(diff)  
        : "r" (count), "r" (num)
        
        );

        result=read_msr(fd,MSR_PKG_ENERGY_STATUS);
        package_after[0]=(double)result*cpu_energy_units[0];

        result=read_msr(fd,MSR_PP0_ENERGY_STATUS);
        pp0_after[0]=(double)result*cpu_energy_units[0];
  
        result=read_msr(fd,MSR_PP1_ENERGY_STATUS);
        pp1_after[0]=(double)result*cpu_energy_units[0];

        result=read_msr(fd,MSR_DRAM_ENERGY_STATUS);
        dram_after[0]=(double)result*dram_energy_units[0];
        close(fd);

  
        //printf("data: \t%u\n", diff);
        data[n] = diff;
        package_pow[n] = package_after[0] - package_before[0];
        pp0_pow[n] = pp0_after[0] - pp0_before[0];
        pp1_pow[n] = pp1_after[0] - pp1_before[0];
        dram_pow[n] = dram_after[0] - dram_before[0];
        //printf("\t\tPackage power: %.6fJ\n",package_pow[n]);
        //printf("\t\tPowerPlane0 (cores): %.6fJ\n",pp0_pow[n]);
        //printf("\t\tPowerPlane1 (GPU): %.6fJ\n",pp1_pow[n]);
        //printf("\t\tDRAM : %.6fJ\n",dram_pow[n]);



    
      }

    for (n = 0; n < MEASURE_ROUND; n++) {
        // fprintf(fp, "%lu\t", data[n]);
      sum += data[n];
      package_sum += package_pow[n];
      pp0_sum += pp0_pow[n];
      pp1_sum += pp1_pow[n];
      dram_sum += dram_pow[n];

      //printf("data: \t%d\n", data[n]);

    }
    pkg_avg = package_sum / MEASURE_ROUND;
    pp0_avg = pp0_sum / MEASURE_ROUND;
    pp1_avg = pp1_sum / MEASURE_ROUND;
    dram_avg = dram_sum / MEASURE_ROUND;
    avg = sum / MEASURE_ROUND;
    printf("avg:\t%d\n", avg);
    printf("Pkg power: \t%.6f\n",pkg_avg);
    printf("pp0 power: \t%.6f\n",pp0_avg);
    printf("pp1 power: \t%.6f\n",pp1_avg);
    printf("dram power: \t%.6f\n",dram_avg);



    return NULL;
}

// Multi-threaded implementation of mandelbrot set image generation.
// Multi-threading performed via pthreads.
void arrayThread(
    int numThreads)
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

    for (int i=1; i<numThreads; i++)
    {
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
int numThreads = 12;
arrayThread(numThreads);
return 0;
}

