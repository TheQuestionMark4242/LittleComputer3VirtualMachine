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


int main(int argc, const char* argv[]) {
    // If the number of arguments provided is not sufficient,
    // we print a user guide on how to use it
    if(argc < 2) {
        printf("lc3 [image-file] ...\n");
        exit(2);
    }

    for(int i = 1; i < argc; i++) if(!read_image(argv[i])){
        printf("Failed to load image: %s\n", argv[i]);
        exit(1);
    }

    // Initialize the conditon flag to 0
    registers[R_COND] = FL_ZRO;

    // Set the program counter to its starting position 
    // The default is 0x3000 for some reason 
    enum { PC_START = 0x3000};
    registers[R_PC] = PC_START;

    int running = 1;
    while(running) {
        uint16_t instruction = memory_read(registers[R_PC]++);
        uint16_t op = instruction >> 12;

        switch(op) {
            case OP_BR:
                break;
            case OP_ADD:
                break;
            case OP_LD:
                break;
            case OP_ST:
                break;
            case OP_JSR:
                break;
            case OP_AND:
                break;
            case OP_LDR:
                break;
            case OP_STR:
                break;
            case OP_LDI:
                break;
            case OP_STI:
                break;
            case OP_JMP:
                break;
            case OP_LEA:
                break;
            case OP_TRAP:
                break;
            case OP_NOT:
                break;
            case OP_RES:
            case OP_RTI:
                break;
            default:
                break;
        }
    }
    return 0;
}