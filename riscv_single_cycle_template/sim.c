#include <stdio.h>
#include "shell.h"

// Local variables for instruction handling
uint32_t current_instruction;
uint32_t decoded_opcode;
uint32_t decoded_rd;
uint32_t decoded_funct3;
uint32_t decoded_rs1;
uint32_t decoded_rs2;
uint32_t decoded_funct7;

void fetch() {
    current_instruction = mem_read_32(CURRENT_STATE.PC);
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    printf("Fetched instruction: 0x%08x\n", current_instruction);
}

void decode() {
    decoded_opcode = current_instruction & 0x7F;
    decoded_rd = (current_instruction >> 7) & 0x1F;
    decoded_funct3 = (current_instruction >> 12) & 0x07;
    decoded_rs1 = (current_instruction >> 15) & 0x1F;
    decoded_rs2 = (current_instruction >> 20) & 0x1F;
    decoded_funct7 = (current_instruction >> 25) & 0x7F;
}

void execute() {
    printf("Executing instruction: 0x%08x\n", current_instruction);
    switch(decoded_opcode) {
        case 0x17: // auipc
            NEXT_STATE.REGS[decoded_rd] = CURRENT_STATE.PC + ((current_instruction >> 12) << 12);
            break;
        case 0x13: // I-type (addi, slli)
            switch(decoded_funct3) {
                case 0x0: // addi
                    NEXT_STATE.REGS[decoded_rd] = CURRENT_STATE.REGS[decoded_rs1] + (int32_t)(current_instruction >> 20);
                    break;
                case 0x1: // slli
                    NEXT_STATE.REGS[decoded_rd] = CURRENT_STATE.REGS[decoded_rs1] << ((current_instruction >> 20) & 0x1F);
                    break;
            }
            break;
        case 0x23: // S-type (sw)
            switch(decoded_funct3) {
                case 0x2: // sw
                    {
                        int32_t imm = ((current_instruction >> 25) << 5) | ((current_instruction >> 7) & 0x1F);
                        int32_t address = CURRENT_STATE.REGS[decoded_rs1] + imm;
                        mem_write_32(address, CURRENT_STATE.REGS[decoded_rs2]);
                    }
                    break;
            }
            break;
        case 0x33: // R-type (add, slt)
            switch(decoded_funct3) {
                case 0x0: // add
                    NEXT_STATE.REGS[decoded_rd] = CURRENT_STATE.REGS[decoded_rs1] + CURRENT_STATE.REGS[decoded_rs2];
                    break;
                case 0x2: // slt
                    NEXT_STATE.REGS[decoded_rd] = (int32_t)CURRENT_STATE.REGS[decoded_rs1] < (int32_t)CURRENT_STATE.REGS[decoded_rs2];
                    break;
            }
            break;
        case 0x6F: // UJ-type (jal)
            {
                int32_t offset = ((current_instruction & 0x80000000) >> 11) | // sign bit
                                 ((current_instruction & 0x7FE00000) >> 20) | // bits 10:1
                                 ((current_instruction & 0x00100000) >> 9)  | // bit 11
                                 ((current_instruction & 0x000FF000));        // bits 19:12
                NEXT_STATE.REGS[decoded_rd] = CURRENT_STATE.PC + 4;
                NEXT_STATE.PC = CURRENT_STATE.PC + (offset << 1);
            }
            break;
        case 0x63: // SB-type (bne)
            switch(decoded_funct3) {
                case 0x1: // bne
                    {
                        int32_t offset = ((current_instruction & 0x80000000) >> 19) | 
                                         ((current_instruction & 0x00000080) << 4) |
                                         ((current_instruction >> 20) & 0x7E0) | 
                                         ((current_instruction >> 7) & 0x1E);
                        if (CURRENT_STATE.REGS[decoded_rs1] != CURRENT_STATE.REGS[decoded_rs2]) {
                            NEXT_STATE.PC = CURRENT_STATE.PC + offset;
                        }
                    }
                    break;
            }
            break;
    }
}

void process_instruction()
{
  /* execute one instruction here. You should use CURRENT_STATE and modify
   * values in NEXT_STATE. You can call mem_read_32() and mem_write_32() to
   * access memory. */
  fetch();
  decode();
  execute();
}
