#include <stdio.h>

#define NUM16   0xFFFF
#define NUM32   0xFFFFFFFF
#define NUM64   0xFFFFFFFFFFFFFFFF


unsigned short add_unsigned_16(unsigned short a, unsigned short b) {
    unsigned short result;

    asm volatile (
        "addw %2, %0;"         // The `addw` instruction adds the two 16-bit unsigned integers.
        : "=r" (result)        // Output in `result`.
        : "0" (a), "r" (b)     // Inputs. The `0` constraint means to use the same register as output `result`.
    );

    return result;
}



unsigned int add_unsigned(unsigned int a, unsigned int b) {
    unsigned int result;

    asm volatile (
        "addl %2, %0;"       // The `addl` instruction adds the two 32-bit integers.
        : "=r" (result)      // Output in `result`.
        : "0" (a), "r" (b)   // Inputs. The `0` constraint means to use the same register as output `result`.
    );

    return result;
}


typedef unsigned long long uint64_t;

uint64_t add_uint64(uint64_t a, uint64_t b) {
    uint64_t result;

    asm volatile (
        "addq %2, %0"          // `addq` instruction adds the two 64-bit integers.
        : "=r" (result)        // Output in `result`.
        : "0" (a), "r" (b)     // Inputs. "0" means to use the same register as the output.
    );

    return result;
}


uint64_t shift_left(uint64_t value, unsigned int shift_count) {

    asm volatile (
        "shl %%cl, %0"    // Shift %0 (val) by cl (shift_amount) bits to the left
        : "+r"(value)       // Output operand (%0), the "+r" means it's also an input and a register operand
        : "c"(shift_count) // Input operand (%1), put it in cl register
    );


    return value;
}





int main() {

    uint64_t value = NUM32;
    unsigned int shift_count = 1;

    while(1){
//    uint64_t a = NUM16, b = NUM16;
//    uint64_t sum16 = add_uint64(a, b);
//    printf("%llu + %llu = %llu\n", a, b, sum16);

//    uint64_t c = NUM32, d = NUM32;
//    uint64_t sum32 = add_uint64(c, d);
//    printf("%llu + %llu = %llu\n", c, d, sum32);


//    uint64_t e = NUM64, f = NUM64;
//    uint64_t sum64 = add_uint64(e, f);
 //   printf("%llu + %llu = %llu\n", e, f, sum64);
 
      uint64_t shifted_value = shift_left(value, shift_count);
//      printf("%llu << %u = %llx\n", value, shift_count, shifted_value);
    }
      return 0;
  }




