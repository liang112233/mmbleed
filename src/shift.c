#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <inttypes.h>

// it is intel in gcc now 
#define MEASURE_ROUND 100000

void shift(uint64_t value, uint64_t shift_amount) {
  uint64_t diff;
  FILE *fp; 
 // unsigned long tdiff;
  int data[MEASURE_ROUND];

  asm volatile(
    "mov %1, %%r13 \t\n" //load count to rax
    "mov %2, %%r14 \t\n" //load num to rbx
    "mov $1500000000000, %%r15 \t\n" //load loop times
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
    "shl  $32,   %%rdx  \t\n"
    "or   %%rdx, %%rax  \t\n"
    "mov  %%rax, %%rcx  \t\n"
    //"movl %%eax, %%ecx  \t\n"
    //"movl %%edx, %%ebx \t\n"
    "loop_start:\n\t"
    "shlx %%r13, %%r14, %%rdx \t\n"
    "shlx %%r13, %%r14, %%rsi \t\n"
    "shrx %%r13, %%r14, %%rdi \t\n"
    "shrx %%r13, %%r14, %%r8 \t\n"
    "shlx %%r13, %%r14, %%r9 \t\n"
    "shlx %%r13, %%r14, %%r10 \t\n"
    "shrx %%r13, %%r14, %%r11 \t\n"
    "shrx %%r13, %%r14, %%r12 \t\n"
    "dec  %%r15     \t\n"
    "jnz  loop_start\t\n"
    //"jmp loop_start\n\t"
    "rdtsc              \t\n"
    "shl  $32,   %%rdx  \t\n"
    "or   %%rdx, %%rax  \t\n"
    "sub  %%rcx, %%rax  \t\n"
    "mov  %%rax, %0     \t\n"
    //"rdtsc            \n\t"
    //"subl %%ecx, %%eax  \n\t"
    //"movl %%eax, %0     \n\t"

    : "=r"(diff) 
    : "r" (shift_amount), "r" (value)
  );

   printf("data: %" PRIu64"\n", diff);


//    fp = fopen("./data/1", "w");
//    fprintf(fp, "%PRIu64\t", diff);
//    fclose(fp);

    
}




int
main(int argc, char *argv[])
{
  uint64_t count, num;
  count = 155555;
  num = 0xFFFFFFFF0000;
  shift(num, count);

  return 0;
}

