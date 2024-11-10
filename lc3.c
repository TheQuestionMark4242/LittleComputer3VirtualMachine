#include <stdint.h>
#include <stdio.h>
//Constructing the memory of the computer 
// The LC3 computer has 2^16 = 65,536 memory locations each 
// storing a 16-bit value 

#define MEMORY_MAX (1 << 16)
uint16_t memory[MEMORY_MAX]; 

//Creating registers
// 10 registers each with 16 bits of memory:
//  1. 8 General purpose registers named R_0 to R_7
//  2. A program counter 
//  3. A condition flag register which will contain information about the last calculation 

#define R_COUNT 10
enum {
    R_R0,
    R_R1,
    R_R2,
    R_R3,
    R_R4,
    R_R5,
    R_R6,
    R_R7,
    R_PC,
    R_COND
};
uint16_t registers[R_COUNT];

int main() {
    printf("%d", R_R0);
    return 0;
}