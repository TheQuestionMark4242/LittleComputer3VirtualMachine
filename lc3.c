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

// An instruction is a command that tells the CPU to perform a certain fundamental operation 
// Each instruction takes in an opcode and a set of parameters
// The set of instructions forms what is called an instruction set. 

enum {
    OP_BR = 0, /* branch */
    OP_ADD,    /* add  */
    OP_LD,     /* load */
    OP_ST,     /* store */
    OP_JSR,    /* jump register */
    OP_AND,    /* bitwise and */
    OP_LDR,    /* load register */
    OP_STR,    /* store register */
    OP_RTI,    /* unused */
    OP_NOT,    /* bitwise not */
    OP_LDI,    /* load indirect */
    OP_STI,    /* store indirect */
    OP_JMP,    /* jump */
    OP_RES,    /* reserved (unused) */
    OP_LEA,    /* load effective address */
    OP_TRAP    /* execute trap */
};

// Condition flags store some information in the 
// R_COND register to check logical conditions on the last performed 
// operation 

enum {
    FL_POS = 1 << 0,
    FL_ZRO = 1 << 1,
    FL_NEG = 1 << 2
};


int main() {
    return 0;
}