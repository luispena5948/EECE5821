// RISCV pipeline timing simulator by Archange Lombo, Luis Pena, and Chad Aboud HW4

#include "pipe.h"
#include "shell.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

/* global pipeline state */
extern CPU_State CURRENT_STATE;

Pipe_Reg_IFtoDE Reg_IFtoDE;
Pipe_Reg_DEtoEX Reg_DEtoEX;
Pipe_Reg_EXtoMEM Reg_EXtoMEM;
Pipe_Reg_MEMtoWB Reg_MEMtoWB;

int should_stall = 0;  // Define and initialize

void pipe_init() {
    memset(&CURRENT_STATE, 0, sizeof(CPU_State));
    CURRENT_STATE.PC = 0x00000000;
    should_stall = 0;  // Initialize stall flag
}

void pipe_cycle() {
    pipe_stage_wb();
    pipe_stage_mem();
    pipe_stage_execute();
    pipe_stage_decode();
    pipe_stage_fetch();
}

/*void pipe_stage_fetch() {
    if (!should_stall) {
        Reg_IFtoDE.instr = mem_read_32(CURRENT_STATE.PC);
        Reg_IFtoDE.PC = CURRENT_STATE.PC;
        CURRENT_STATE.PC += 4;  // increment PC
    }
}
*/

void pipe_stage_fetch() {
    uint32_t instruction = mem_read_32(CURRENT_STATE.PC);
    Reg_IFtoDE.instr = instruction;
    CURRENT_STATE.PC += 4;  // increment PC by 4 for the next instruction
}


void pipe_stage_decode() {
    uint32_t instr = Reg_IFtoDE.instr;
    uint32_t opcode = instr & 0x7F;
    Reg_DEtoEX.opcode = opcode;
    Reg_DEtoEX.rs1 = (instr >> 15) & 0x1F;
    Reg_DEtoEX.rs2 = (instr >> 20) & 0x1F;
    Reg_DEtoEX.rd = (instr >> 7) & 0x1F;
    Reg_DEtoEX.func3 = (instr >> 12) & 0x7;
    Reg_DEtoEX.func7 = (instr >> 25) & 0x7F;
    Reg_DEtoEX.immediate = sign_extend_i_type(instr);  // Assuming sign_extend_i_type() is implemented
}

void pipe_stage_execute() {
    if (Reg_DEtoEX.opcode == OPCODE_R_TYPE && Reg_DEtoEX.func3 == FUNCT3_ADD && Reg_DEtoEX.func7 == FUNCT7_ADD) {
        int32_t val1 = CURRENT_STATE.REGS[Reg_DEtoEX.rs1];
        int32_t val2 = CURRENT_STATE.REGS[Reg_DEtoEX.rs2];
        Reg_EXtoMEM.result = val1 + val2;
        Reg_EXtoMEM.rd = Reg_DEtoEX.rd;
    }
}

void pipe_stage_mem() {
    Reg_MEMtoWB.result = Reg_EXtoMEM.result;  // Directly pass the result for non-memory ops
    Reg_MEMtoWB.rd = Reg_EXtoMEM.rd;
}

void pipe_stage_wb() {
    CURRENT_STATE.REGS[Reg_MEMtoWB.rd] = Reg_MEMtoWB.result;
}

int32_t sign_extend_i_type(uint32_t instr) {
    // Assuming immediate is at bits 20-31 for an I-type instruction
    int32_t immediate = (instr >> 20) & 0xFFF;  // Extract 12-bit immediate
    if (immediate & 0x800)  // Check if immediate is negative
        immediate |= 0xFFFFF000;  // Sign-extend to 32 bits
    return immediate;
}
