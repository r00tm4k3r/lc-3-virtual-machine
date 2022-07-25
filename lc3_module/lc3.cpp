#include "lc3.hpp"

using namespace std;

// @private methods
void lc3vm::read_image_file(FILE *file)
{
    /* the origin tells us where in memory to place the image */
    uint16_t origin_size;
    fread(&origin_size, sizeof(origin_size), 1, file); // во-первых: что он делает; во-вторых как его сделать более с++?
    origin_size = swap16(origin_size);

    /* we know the maximum file size so we only need one fread */
    uint16_t max_read = MEMORY_MAX - origin_size;
    uint16_t *p = memory + origin_size;
    size_t read = fread(p, sizeof(uint16_t), max_read, file);

    /* swap to little endian */
    while (read-- > 0)
    {
        *p = swap16(*p);
        ++p;
    }
}

int lc3vm::read_image(const char *file_path)
{
    FILE *file = fopen(file_path, "rb");
    if (!file)
        return 0;
    read_image_file(file);
    fclose(file);
    return 1;
}

uint16_t lc3vm::swap16(uint16_t x)
{
    return (x << 8) | (x >> 8);
}

void lc3vm::mem_write(uint16_t address, uint16_t val)
{
    memory[address] = val;
}

uint16_t lc3vm::mem_read(uint16_t address)
{
    if (address == MR_KBSR)
    {
        if (check_key())
        {
            memory[MR_KBSR] = (1 << 15); // ЧТО ТУТ ПРОИСХОДИТ????
            memory[MR_KBDR] = getchar();
        }
        else
            memory[MR_KBSR] = 0;
        return memory[address];
    }
}

uint16_t lc3vm::check_key()
{
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    return select(1, &readfds, NULL, NULL, &timeout) != 0;
}

uint16_t lc3vm::sign_extend(uint16_t x, int bit_count)
{
    if ((x >> (bit_count - 1)) & 1)
    {
        x |= (0xFFFF << bit_count);
    }
    return x;
}

void lc3vm::update_flags(uint16_t r) //можно сделать лучше

{
    if (reg[r] == 0)
    {
        reg[R_COND] = FL_ZRO;
    }
    else if (reg[r] >> 15) /* a 1 in the left-most bit indicates negative */
    {
        reg[R_COND] = FL_NEG;
    }
    else
    {
        reg[R_COND] = FL_POS;
    }
}

void lc3vm::op_br(uint16_t instr)
{
    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
    uint16_t cond_flag = (instr >> 9) & 0x7;
    if (cond_flag & reg[R_COND])
    {
        reg[R_PC] += pc_offset;
    }
}

void lc3vm::op_add(uint16_t instr)
{
    uint16_t dr = (instr >> 9) & 0x7;
    uint16_t sr1 = (instr >> 6) & 0x7;
    if ((instr >> 5) & 0x1)
        reg[dr] = reg[sr1] + sign_extend(instr & 0x1F, 5);
    else
    {
        uint16_t sr2 = instr & 0x7;
        reg[dr] = reg[sr1] + reg[sr2];
    }
    update_flags(dr);
}

void lc3vm::op_ld(uint16_t instr)
{
    uint16_t dr = (instr >> 9) & 0x7;
    uint16_t offset = sign_extend(instr & 0x01FF, 9);
    reg[dr] = mem_read(reg[R_PC] + offset);
}

void lc3vm::op_st(uint16_t instr)
{
    uint16_t sr = (instr >> 9) & 0x7;
    uint16_t offset = sign_extend(instr & 0x01FF, 9);
    mem_write(mem_read(reg[R_PC] + offset), reg[sr]);
}

void lc3vm::op_jsr(uint16_t instr)
{
    if ((instr >> 11) & 0x1)
    {
        uint16_t offset = sign_extend(instr & 0x07FF, 11);
        reg[R_PC] += offset;
    }
    else
    {
        uint16_t base_r = (instr >> 6) & 0x7;
        reg[R_PC] = reg[base_r];
    }
}

void lc3vm::op_and(uint16_t instr)
{
    uint16_t dr = (instr >> 9) & 0x7;
    uint16_t sr1 = (instr >> 6) & 0x7;
    if ((instr >> 5) & 0x1)
        reg[dr] = reg[sr1] & sign_extend(instr & 0x1F, 5);
    else
    {
        uint16_t sr2 = instr & 0x7;
        reg[dr] = reg[sr1] & reg[sr2];
    }
    update_flags(dr);
}

void lc3vm::op_ldr(uint16_t instr)
{
    uint16_t dr = (instr >> 9) & 0x7;
    uint16_t base_r = (instr >> 6) & 0x7;
    uint16_t offset = sign_extend(instr & 0x3F, 6);
    reg[dr] = mem_read(reg[base_r] + offset);
    update_flags(dr);
}

void lc3vm::op_str(uint16_t instr)
{
    uint16_t dr = (instr >> 9) & 0x7;
    uint16_t base_r = (instr >> 6) & 0x7;
    uint16_t offset = sign_extend(instr & 0x3F, 6);
    mem_write(reg[base_r] + offset, reg[dr]);
}

void lc3vm::op_rti(uint16_t instr)
{
    // ffffffffffuck
}

void lc3vm::op_not(uint16_t instr)
{
    uint16_t dr = (instr >> 9) & 0x7;
    uint16_t sr = (instr >> 6) & 0x7;
    reg[dr] = ~reg[sr];
    update_flags(dr);
}

void lc3vm::op_ldi(uint16_t instr)
{
    uint16_t dr = (instr >> 9) & 0x7;
    uint16_t offset = sign_extend(instr & 0x01FF, 9);

    reg[dr] = mem_read(mem_read(reg[R_PC] + offset));
    update_flags(dr);
}

void lc3vm::op_sti(uint16_t instr)
{
    uint16_t dr = (instr >> 9) & 0x7;
    uint16_t offset = sign_extend(instr & 0x01FF, 9);
    mem_write(mem_read(reg[R_PC] + offset), reg[dr]);
}

void lc3vm::op_jmp(uint16_t instr)
{
    uint16_t base_r = (instr >> 6) & 0x7;
    reg[R_PC] = reg[base_r];
}

void lc3vm::op_lea(uint16_t instr)
{
    uint16_t dr = (instr >> 9) & 0x7;
    uint16_t offset = sign_extend(instr & 0x01FF, 9);
    reg[dr] = reg[R_PC] + offset;
    update_flags(dr);
}

void lc3vm::op_trap(uint16_t instr)
{
    uint16_t trapvector = instr & 0xFF;

    switch (trapvector)
    {
    case TRAP_GETC:
    {
        reg[R_R0] = (uint16_t)getchar(); // есть ли в плюсах альтернатива getchar?
        update_flags(R_R0);
        break;
    }
    case TRAP_OUT:
    {
        putc((char)reg[R_R0], stdout);
        fflush(stdout);
        break;
    }
    case TRAP_PUTS:
    {

        break;
    }
    case TRAP_IN:
    {
        cout << "Enter character: ";
        char c = getchar();
        putc(c, stdout);
        fflush(stdout); // есть ли альтернатива?
        reg[R_R0] = (uint16_t)c;
        update_flags(R_R0);
        break;
    }
    case TRAP_PUTSP:
    {
        uint16_t *c = memory + reg[R_R0];
        while (*c)
        {
            putc((*c) & 0xFF, stdout);
            putc((*c) >> 8, stdout);
            c++;
        }
        fflush(stdout);
        break;
    }
    case TRAP_HALT:
    {
        puts("HALT");
        fflush(stdout);
        running = 0;
        break;
    }
    }
}

lc3vm::lc3vm()
{
    reg[R_COND] = FL_ZRO;
    reg[R_PC] = 0X3000;
    running = true;
    cout << "LC3 VM init:" << reg[R_PC] << " " << reg[R_COND] << " " << endl;
}

void lc3vm::run(const char *path)
{
    if (!read_image(path))
    {
        cout << "Failed to load image: " << path << endl;
        exit(1);
    }
    while (running)
    {

        uint16_t instr = mem_read(reg[R_PC]++);
        uint16_t op = instr >> 12;

        switch (op) // здесь устроить цикл пробега по map в поисках функции выполнения?
        {
        case OP_BR:
        {
            op_br(instr);
            break;
        }
        case OP_ADD:
        {
            op_add(instr);
            break;
        }
        case OP_LD:
        {
            op_ld(instr);
            break;
        }
        case OP_ST:
        {
            op_st(instr);
            break;
        }
        case OP_JSR:
        {
            op_jsr(instr);
            break;
        }
        case OP_AND:
        {
            op_and(instr);
            break;
        }
        case OP_LDR:
        {
            op_ldr(instr);
            break;
        }
        case OP_STR:
        {
            op_str(instr);
            break;
        }
        case OP_NOT:
        {
            op_not(instr);
            break;
        }
        case OP_LDI:
        {
            op_ldi(instr);
            break;
        }
        case OP_STI:
        {
            op_sti(instr);
            break;
        }
        case OP_JMP:
        {
            op_jmp(instr);
            break;
        }
        case OP_LEA:
        {
            op_lea(instr);
            break;
        }
        case OP_TRAP:
        {
            op_trap(instr);
            break;
        }
        case OP_RES:
        case OP_RTI:
        default:
        {
            cout << "BAD OPCODE: " << op << endl;
            throw exception(); // что тут будет происходить?
        }
        break;
        }
    }
}
