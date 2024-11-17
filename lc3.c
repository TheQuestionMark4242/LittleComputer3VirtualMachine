#include <stdio.h>
#include <stdint.h>
#include <signal.h>
/* windows only */
#include <Windows.h>
#include <conio.h>  // _kbhit

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

enum {
    
    // Special registers not accesible from the normal 
    // register table, instead stored at a specific location in 
    // memory, commonly used to interact with special hardware 
    // devices. 
    
    MR_KBSR = 0xFE00,  // Keyboard Status
    MR_KBDR = 0xFE02   // Keyboard Data 
};

void memory_write(uint16_t address, uint16_t value) {
    memory[address] = value;
}

uint16_t memory_read(uint16_t address) {
    if(address != MR_KBSR) {
        return memory[address];
    }
    if(check_key()) {
        memory[MR_KBSR] = (1 << 15);
        memory[MR_KBDR] = getchar();
    }
    else {
        memory[MR_KBSR] = 0;
    }
    return memory[address];
}

HANDLE hStdin = INVALID_HANDLE_VALUE;
DWORD fdwMode, fdwOldMode;

void disable_input_buffering(){
    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(hStdin, &fdwOldMode); /* save old mode */
    fdwMode = fdwOldMode
            ^ ENABLE_ECHO_INPUT  /* no input echo */
            ^ ENABLE_LINE_INPUT; /* return when one or
                                    more characters are available */
    SetConsoleMode(hStdin, fdwMode); /* set new mode */
    FlushConsoleInputBuffer(hStdin); /* clear buffer */
}

void restore_input_buffering(){
    SetConsoleMode(hStdin, fdwOldMode);
}

uint16_t check_key(){
    return WaitForSingleObject(hStdin, 1000) == WAIT_OBJECT_0 && _kbhit();
}

void handle_interrupt(int signal) {
    restore_input_buffering();
    printf("\n");
    exit(-2);
}

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

uint16_t sign_extend(uint16_t x, int bit_count) {
    if(x >> (bit_count - 1) & 1) {
        x |= (0xFFFF << bit_count);
    }
    return x;
}

void update_flags(uint16_t r) {
    if(registers[r] == 0) {
        registers[R_COND] = FL_ZRO;
    }
    else return (registers[r] >> 15 ? FL_NEG : FL_POS);
}

void op_add(uint16_t instruction) {
    uint16_t destination_register = (instruction >> 9) & 0x7;
    uint16_t source_register1 = (instruction >> 6) & 0x7;
    uint16_t immediate_flag = (instruction >> 5) & 0x1;

    if(immediate_flag) {
        // In immediate mode
        uint16_t immediate = sign_extend(instruction & 0x1F, 5);
        registers[destination_register] = registers[source_register1] + immediate;
    }
    else {
        uint16_t source_register2 = instruction & 0x7;
        registers[destination_register] = registers[source_register1] + source_register2;
    }
    update_flags(destination_register);
}

void op_load(uint16_t instruction) {
    uint16_t destination_register = (instruction >> 9) & 0x7;
    uint16_t pc_offset = sign_extend(instruction & 0x1FF, 9);
    registers[destination_register] = memory_read(registers[R_PC] + pc_offset); 
    update_flags(destination_register);
}

void op_load_indirect(uint16_t instruction) {
    uint16_t destination_register = (instruction >> 9) & 0x7;
    uint16_t pc_offset = sign_extend(instruction & 0x1FF, 9);
    registers[destination_register] = memory_read(memory_read(registers[R_PC] + pc_offset));
    update_flags(destination_register);
}

void op_and(uint16_t instruction) {
    uint16_t destination_register = (instruction >> 9) & 0x7;
    uint16_t source_register1 = (instruction >> 6) & 0x7;
    uint16_t immediate_flag = (instruction >> 5) & 0x1;
    if(immediate_flag) {
        uint16_t immediate = sign_extend(instruction & 0x1F, 5); 
        registers[destination_register] = registers[source_register1] & immediate;
    }
    else {
        uint16_t source_register2 = instruction & 0x7;
        registers[destination_register] = registers[source_register1] & registers[source_register2];
    }
    update_flags(destination_register);
}

void op_conditional_branch(uint16_t instruction) {
    // Justin Meiners presents a more convenient way
    // Instead of checking these separately
    // one can just check if the 3 bits togetger anded with the 
    // condition flag is non-zero.
    uint16_t n = (instruction >> 11) & 0x1;
    uint16_t z = (instruction >> 10) & 0x1;
    uint16_t p = (instruction >> 9) & 0x1;
    uint16_t pc_offset = sign_extend(instruction & 0x1FF, 9);
    if((n && registers[R_COND] == FL_NEG) || (z && registers[R_COND] == FL_ZRO) || (p && registers[R_COND] == FL_POS)) {
        registers[R_PC] = registers[R_PC] + pc_offset;
    }
}

void op_jump(uint16_t instruction) {
    //Should also handle the RET instruction when the base_register is R_7.
    uint16_t base_register = (instruction >> 6) & 0x7;
    registers[R_PC] = registers[base_register];
}

void op_jump_to_subroutine(uint16_t instruction) {
    registers[R_R7] = registers[R_PC];
    if(instruction >> 11 & 0x1) {
        uint16_t pc_offset = sign_extend(instruction & 0x7FF, 11);
        registers[R_PC] += pc_offset;
    }
    else {
        uint16_t base_register = (instruction >> 6) & 0x7;
        registers[R_PC] = registers[base_register];
    }
}


void op_store(uint16_t instruction) {
    uint16_t source_register = instruction >> 9 & 0x7;
    uint16_t pc_offset = sign_extend(instruction & 0x1FF, 9);
    memory_write(registers[R_PC] + pc_offset, registers[source_register]);
}

void op_store_indirect(uint16_t instruction) {
    uint16_t source_register = instruction >> 9 & 0x7;
    uint16_t pc_offset = sign_extend(instruction & 0x1FF, 9);
    memory_write(memory_read(registers[R_PC] + pc_offset, registers[source_register]));
}

void op_load_base_plus_offset(uint16_t instruction) {
    uint16_t destination_register = (instruction >> 9) & 0x7;
    uint16_t base_register = (instruction >> 6) & 0x7; 
    uint16_t offset = sign_extend(instruction & 0x3F, 6);
    registers[destination_register] = memory_read(registers[base_register] + offset);
    update_flags(destination_register);
}

void op_load_effective_address(uint16_t instruction) {
    uint16_t destination_register = (instruction >> 9) & 0x7;
    uint16_t pc_offset = sign_extend(instruction & 0x1FF, 9);
    registers[destination_register] = registers[R_PC] + pc_offset;
    update_flags(destination_register);
}

void op_store_base_plus_offset(uint16_t instruction) {
    uint16_t source_register = (instruction >> 9) & 0x7;
    uint16_t base_register = (instruction >> 6) & 0x7;
    uint16_t offset = sign_extend(instruction & 0x3F, 6);
    memory_write(registers[base_register] + offset, registers[source_register]);
}

enum {
    TRAP_GETC = 0x20,  // Get a character from the keyboard, not echoed to the terminal 
    TRAP_OUT = 0x21,   // Output a character 
    TRAP_PUTS = 0x22,  // Output a word string 
    TRAP_IN = 0x23,    // Get a character from the keyboard, echoed to the terminal 
    TRAP_PUTSP = 0x24, // Output a byte string
    TRAP_HALT = 0x25   // Halt the program 
};

void op_system_call(uint16_t instruction) {
    uint16_t trap_vector = instruction & 0xFF; //Implicitly zero extended here 
    registers[R_R7] = registers[R_PC];
    // Execute system calls, mostly for dealing with I/O
    switch (instruction & 0xFF) {
        case TRAP_GETC:
            registers[R_R0] = (uint16_t) getchar();
            update_flags(R_R0);
            break;
        case TRAP_OUT:
            putc((char) registers[R_R0], stdout);
            fflush(stdout);
            break;
        case TRAP_PUTS:
            uint16_t* c = memory + registers[R_R0];
            while(*c) {
                putc((char) *c, stdout);
                ++c;
            }
            fflush(stdout);
            break;
        case TRAP_IN:
            printf("Enter a character: ");
            char c = getchar();
            putc(c, stdout);
            registers[R_R0] = (uint16_t) c;
            update_flags(R_R0);
            break;
        case TRAP_PUTSP:
            uint16_t* c = memory + registers[R_R0];
            while(*c) {
                char char1 = (*c) & 0xFF;
                putc(char1, stdout);
                char char2 = (*c) >> 8;
                if (char2) putc(char2, stdout);
                ++c;
            }
            fflush(stdout);
            break;
        case TRAP_HALT:
            puts("HALT");
            fflush(stdout);
            break;
    }
}

void op_not(uint16_t instruction) {
    uint16_t destination_register = (instruction >> 9) & 0x7;
    uint16_t source_register = (instruction >> 6) & 0x7;
    registers[destination_register] = ~registers[source_register];
    update_flags(destination_register);
}

uint16_t swap16(uint16_t x) {
    return (x << 8) | (x >> 8);
}

void read_image_file(FILE* file) {
    /*
        Each LC3 program starts with 16 bits specifying the 
        memory address where the program should start.
    */
    uint16_t origin;
    fread(&origin, sizeof(origin), 1, file);
    origin = swap16(origin);

    uint16_t max_read = MEMORY_MAX - origin;
    uint16_t *p = memory + origin;
    size_t read = fread(p, sizeof(uint16_t), max_read, file);

    while(read-- > 0) {
        *p = swap16(*p);
        ++p;
    }

}

int read_image(const char* image_path) {
    FILE* file = fopen(image_path, "rb");
    if(!file) {
        return 0;
    }
    read_image_file(file);
    fclose(file);
    return 1;
}

int main(int argc, const char* argv[]) {
    signal(SIGINT, handle_interrupt);
    disable_input_buffering();

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
                op_conditional_branch(instruction);
                break;
            case OP_ADD:
                op_add(instruction);
                break;
            case OP_LD:
                op_load(instruction);
                break;
            case OP_ST:
                op_store(instruction);
                break;
            case OP_JSR:
                op_jump_to_subroutine(instruction);
                break;
            case OP_AND:
                op_and(instruction);
                break;
            case OP_LDR:
                op_load_base_plus_offset(instruction);
                break;
            case OP_STR:
                op_store_base_plus_offset(instruction);
                break;
            case OP_LDI:
                op_load_indirect(instruction);
                break;
            case OP_STI:
                op_store_indirect(instruction);
                break;
            case OP_JMP:
                op_jump(instruction);
                break;
            case OP_LEA:
                op_load_effective_address(instruction);
                break;
            case OP_TRAP:
                op_system_call(instruction);
                break;
            case OP_NOT:
                op_not(instruction);
                break;
            case OP_RES:
            case OP_RTI:
                abort();
                break;
            default:
                break;
        }
    }

    restore_input_buffering();
    return 0;
}