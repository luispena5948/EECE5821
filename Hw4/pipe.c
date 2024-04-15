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
    Reg_DEtoEX.rd = (instr >> 7) & 0x1F;
    Reg_DEtoEX.func3 = (instr >> 12) & 0x7;
    Reg_DEtoEX.rs1 = (instr >> 15) & 0x1F;
    Reg_DEtoEX.rs2 = (instr >> 20) & 0x1F;
    Reg_DEtoEX.func7 = (instr >> 25) & 0x7F;
    Reg_DEtoEX.immediate = sign_extend_i_type(instr);  // Assuming sign_extend_i_type() is implemented
}

void pipe_stage_execute() {

    //uint32_t current_instruction = Reg_IFtoDE.instr;
/*
    if (Reg_DEtoEX.opcode == OPCODE_R_TYPE && Reg_DEtoEX.func3 == FUNCT3_ADD && Reg_DEtoEX.func7 == FUNCT7_ADD) {
        int32_t val1 = CURRENT_STATE.REGS[Reg_DEtoEX.rs1];
        int32_t val2 = CURRENT_STATE.REGS[Reg_DEtoEX.rs2];
        Reg_EXtoMEM.result = val1 + val2;
        Reg_EXtoMEM.rd = Reg_DEtoEX.rd;
    }
*/
    
    printf("Executing instruction: 0x%08x\n", Reg_IFtoDE.instr); // Print the instruction being executed as a hexadecimal value for debugging.
    switch(Reg_DEtoEX.opcode) { // Switch on the opcode to determine the type of instruction and its operation.
        case 0x17: // Handle 'auipc' instruction.
            // 'auipc' adds the upper immediate to the program counter. The immediate is left-shifted 12 bits
            // and added to the current PC, then stored in the destination register
            CURRENT_STATE.REGS[Reg_DEtoEX.rd] = CURRENT_STATE.PC + ((Reg_IFtoDE.instr >> 12) << 12);
            break;
        case 0x13: // Handle I-type instructions like 'addi' and 'slli'.
            switch(Reg_DEtoEX.func3) {
                case 0x0: // 'addi' adds an immediate to a register.
                    // Sign-extend the immediate value (shift right to position, then cast to signed integer) and add it to rs1.
                    CURRENT_STATE.REGS[Reg_DEtoEX.rd] = CURRENT_STATE.REGS[Reg_DEtoEX.rs1] + Reg_DEtoEX.immediate;
                    break;
            }
            break;
        /**/
        case 0x3: // Handle I-type instructions like lw.
            switch(Reg_DEtoEX.func3) {
                case 0x2: // 'lw' load word to a register with inmidiate offset.
                    // Sign-extend the immediate value (shift right to position, then cast to signed integer) and add it to rs1.
                    int32_t address = CURRENT_STATE.REGS[Reg_DEtoEX.rs1] + Reg_DEtoEX.immediate ;
                    CURRENT_STATE.REGS[Reg_DEtoEX.rd] = mem_read_32(address);
                    
                    break;
            }
            break;
        /**/
        case 0x23: // Handle S-type instructions like 'sw'.
            switch(Reg_DEtoEX.func3) {
                case 0x2: // 'sw' stores a word in memory.
                    // Calculate the effective address by adding the sign-extended immediate to rs1.
                    {
                        int32_t imm = ((Reg_IFtoDE.instr >> 25) << 5) | ((Reg_IFtoDE.instr >> 7) & 0x1F);
                        int32_t address = CURRENT_STATE.REGS[Reg_DEtoEX.rs1] + imm;
                        mem_write_32(address, CURRENT_STATE.REGS[Reg_DEtoEX.rs2]);  // Write the value in rs2 to the calculated memory address.
                    }
                    break;
            }
            break;
        case 0x33: // Handle R-type instructions like 'add' and 'slt'.
            switch(Reg_DEtoEX.func3) {
                case 0x0: 
                    // Add the values in rs1 and rs2, store the result in rd.
                    if(Reg_DEtoEX.func7 == 0x0){ // 'add' adds two registers.
                        CURRENT_STATE.REGS[Reg_DEtoEX.rd] = CURRENT_STATE.REGS[Reg_DEtoEX.rs1] + CURRENT_STATE.REGS[Reg_DEtoEX.rs2];
                    }
                    else if (Reg_DEtoEX.func7 == 0x20){ // 'substract' adds two registers.
                        CURRENT_STATE.REGS[Reg_DEtoEX.rd] = CURRENT_STATE.REGS[Reg_DEtoEX.rs1] - CURRENT_STATE.REGS[Reg_DEtoEX.rs2];
                    }
                    
                    break;
                case 0x2: // 'slt' sets rd to 1 if rs1 < rs2, otherwise 0.
                    // Compare rs1 and rs2 as signed integers, set rd to 1 if rs1 is less than rs2, otherwise 0.
                    CURRENT_STATE.REGS[Reg_DEtoEX.rd] = (int32_t)CURRENT_STATE.REGS[Reg_DEtoEX.rs1] < (int32_t)CURRENT_STATE.REGS[Reg_DEtoEX.rs2];
                    break;
            }
            break;
        
        case 0x63: // Handle SB-type 'blt' instruction.
            switch(Reg_DEtoEX.func3) {
                case 0x4: // 'blt' branches if registers are not equal.
                    {
                        // Calculate the branch offset, considering various parts of the instruction for sign extension and bit positions.
                        int32_t offset = ((Reg_IFtoDE.instr & 0x80000000) >> 19) | 
                                         ((Reg_IFtoDE.instr  & 0x00000080) << 4) |
                                         ((Reg_IFtoDE.instr  >> 20) & 0x7E0) | 
                                         ((Reg_IFtoDE.instr  >> 7) & 0x1E);
                        // Perform the branch if rs1 and rs2 are not equal by updating the PC with the calculated offset.
                        if (CURRENT_STATE.REGS[Reg_DEtoEX.rs1] < CURRENT_STATE.REGS[Reg_DEtoEX.rs2]) {  
                            CURRENT_STATE.PC = CURRENT_STATE.PC + offset;
                        }
                    }
                    break;
            }
            break;

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
