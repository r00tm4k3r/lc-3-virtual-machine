#pragma once
#include <iostream>
#include <cstdint>
#include <csignal>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/termios.h>
#include <sys/mman.h>



#define MEMORY_MAX (1 << 16)

class lc3vm
{
    private:

        enum // Memory Mapped Registers
        {
            MR_KBSR = 0xFE00, /* keyboard status */
            MR_KBDR = 0xFE02  /* keyboard data */
        };

        enum //trap codes   
        {
            TRAP_GETC = 0x20,  /* get character from keyboard, not echoed onto the terminal */
            TRAP_OUT = 0x21,   /* output a character */
            TRAP_PUTS = 0x22,  /* output a word string */
            TRAP_IN = 0x23,    /* get character from keyboard, echoed onto the terminal */
            TRAP_PUTSP = 0x24, /* output a byte string */
            TRAP_HALT = 0x25   /* halt the program */
        };

        enum // registers
        {
            R_R0 = 0,
            R_R1,
            R_R2,
            R_R3,
            R_R4,
            R_R5,
            R_R6,
            R_R7,
            R_PC, //PROGRAM COUNTER
            R_COND,
            R_COUNT
        };

        enum // condition flags
        {
            FL_POS = 1 << 0, /* P */
            FL_ZRO = 1 << 1, /* Z */
            FL_NEG = 1 << 2, /* N */
        };

        enum // opcodes
        {
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

        uint16_t reg[R_COUNT];
        uint16_t memory[MEMORY_MAX];
        bool running;

        struct termios original_tio;


    void read_image_file(FILE* file);
    int read_image(const char* image_path);
    uint16_t swap16(uint16_t x);
    
    uint16_t mem_read(uint16_t address);
    void mem_write(uint16_t address, uint16_t val);

    uint16_t check_key();
    uint16_t lc3vm::sign_extend(uint16_t x, int bit_count);
    void update_flags(uint16_t r);


    //@ opcodes funcs
    void op_br(uint16_t instr);
    void op_add(uint16_t instr);
    void op_ld(uint16_t instr);
    void op_st(uint16_t instr);
    void op_jsr(uint16_t instr);
    void op_and(uint16_t instr);
    void op_ldr(uint16_t instr);
    void op_str(uint16_t instr);
    void op_rti(uint16_t instr);
    void op_not(uint16_t instr);
    void op_ldi(uint16_t instr);
    void op_sti(uint16_t instr);
    void op_jmp(uint16_t instr);
    void op_lea(uint16_t instr);
    void op_trap(uint16_t instr);

    public:
    lc3vm();
    void run(const char* path);
};