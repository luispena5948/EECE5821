//RISCV pipeline timing simulator by Archange Lombo, Luis Pena, and Chad Aboud HW4

#ifndef _PIPE_H_
#define _PIPE_H_

#include "shell.h"
#include "stdbool.h"
#include <limits.h>

#define RISCV_REGS 32
extern int should_stall;  // Declare it as an external variable

#define OPCODE_R_TYPE 0x33  // Example opcode for R-Type
#define FUNCT3_ADD    0x00  // Example funct3 for ADD
#define FUNCT7_ADD    0x20  // Example funct7 for ADD

int32_t sign_extend_i_type(uint32_t instr);

typedef struct CPU_State_Struct {
    uint32_t PC;         /* program counter */
    int32_t REGS[RISCV_REGS]; /* register file. */
    int FLAG_NV;        /* invalid operation */
    int FLAG_DZ;        /* divide by zero */
    int FLAG_OF;        /* overflow */
    int FLAG_UF;        /* underflow */
    int FLAG_NX;        /* inexact */
} CPU_State;

typedef struct Pipe_Reg_IFtoDE {
    uint32_t instr;
    uint32_t PC;
    // fill additional information to store here
} Pipe_Reg_IFtoDE;

typedef struct Pipe_Reg_DEtoEX {
    uint32_t opcode;
    uint32_t rs1;
    uint32_t rs2;
    uint32_t rd;
    uint32_t func3;
    uint32_t func7;
    uint32_t immediate;
    // fill additional information to store here
} Pipe_Reg_DEtoEX;

typedef struct Pipe_Reg_EXtoMEM {
    uint32_t result;
    uint32_t rd;
    // fill additional information to store here
} Pipe_Reg_EXtoMEM;

typedef struct Pipe_Reg_MEMtoWB {
    uint32_t result;
    uint32_t rd;
    // fill additional information to store here
} Pipe_Reg_MEMtoWB;

extern int RUN_BIT;

/* global variable -- pipeline state */
extern CPU_State CURRENT_STATE;

/* called during simulator startup */
void pipe_init();

/* this function calls the others */
void pipe_cycle();

/* each of these functions implements one stage of the pipeline */
void pipe_stage_fetch();
void pipe_stage_decode();
void pipe_stage_execute();
void pipe_stage_mem();
void pipe_stage_wb();

#endif
