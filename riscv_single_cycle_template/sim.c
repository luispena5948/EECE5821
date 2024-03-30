# Group Members: Chad Abboud, Archange Lombo, Luis Pena Mateo
# HW Assignment 3 - Writing a simulator for single-cycle RISC-V datapath

#include <stdio.h>
#include "shell.h"

// Local variables for instruction handling
uint32_t current_instruction; // Holds the current instruction fetched from memory.
uint32_t decoded_opcode; // Holds the opcode part of the instruction.
uint32_t decoded_rd; // Holds the destination register identifier.
uint32_t decoded_funct3; // Holds the 3-bit function field, used in some formats to determine the exact operation.
uint32_t decoded_rs1; // Holds the first source register identifier.
uint32_t decoded_rs2; // Holds the second source register identifier.
uint32_t decoded_funct7; // Holds the 7-bit function field, used in some instruction formats for additional operation details.

// Function to fetch the next instruction from memory.
void fetch() {
    current_instruction = mem_read_32(CURRENT_STATE.PC); // Read the instruction from memory at the address of the current program counter (PC).
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;                // Increment the PC by 4 to move to the next instruction, assuming 32-bit instructions.
    printf("Fetched instruction: 0x%08x\n", current_instruction);
}

// Function to decode the fetched instruction into its components.
void decode() {                                         // Mask and shift operations to extract each part of the instruction.
    decoded_opcode = current_instruction & 0x7F; 
    decoded_rd = (current_instruction >> 7) & 0x1F; 
    decoded_funct3 = (current_instruction >> 12) & 0x07; 
    decoded_rs1 = (current_instruction >> 15) & 0x1F;  
    decoded_rs2 = (current_instruction >> 20) & 0x1F;
    decoded_funct7 = (current_instruction >> 25) & 0x7F;
}

// Function to execute the decoded instruction.
void execute() {
    printf("Executing instruction: 0x%08x\n", current_instruction); // Print the instruction being executed as a hexadecimal value for debugging.
    switch(decoded_opcode) { // Switch on the opcode to determine the type of instruction and its operation.
        case 0x17: // Handle 'auipc' instruction.
            // 'auipc' adds the upper immediate to the program counter. The immediate is left-shifted 12 bits
            // and added to the current PC, then stored in the destination register
            NEXT_STATE.REGS[decoded_rd] = CURRENT_STATE.PC + ((current_instruction >> 12) << 12);
            break;
        case 0x13: // Handle I-type instructions like 'addi' and 'slli'.
            switch(decoded_funct3) {
                case 0x0: // 'addi' adds an immediate to a register.
                    // Sign-extend the immediate value (shift right to position, then cast to signed integer) and add it to rs1.
                    NEXT_STATE.REGS[decoded_rd] = CURRENT_STATE.REGS[decoded_rs1] + (int32_t)(current_instruction >> 20);
                    break;
                case 0x1: // 'slli' shifts a register left by an immediate value.
                    // Shift rs1 left by the lower 5 bits of the immediate value.
                    NEXT_STATE.REGS[decoded_rd] = CURRENT_STATE.REGS[decoded_rs1] << ((current_instruction >> 20) & 0x1F);
                    break;
            }
            break;
        case 0x23: // Handle S-type instructions like 'sw'.
            switch(decoded_funct3) {
                case 0x2: // 'sw' stores a word in memory.
                    // Calculate the effective address by adding the sign-extended immediate to rs1.
                    {
                        int32_t imm = ((current_instruction >> 25) << 5) | ((current_instruction >> 7) & 0x1F);
                        int32_t address = CURRENT_STATE.REGS[decoded_rs1] + imm;
                        mem_write_32(address, CURRENT_STATE.REGS[decoded_rs2]);  // Write the value in rs2 to the calculated memory address.
                    }
                    break;
            }
            break;
        case 0x33: // Handle R-type instructions like 'add' and 'slt'.
            switch(decoded_funct3) {
                case 0x0: // 'add' adds two registers.
                    // Add the values in rs1 and rs2, store the result in rd.
                    NEXT_STATE.REGS[decoded_rd] = CURRENT_STATE.REGS[decoded_rs1] + CURRENT_STATE.REGS[decoded_rs2];
                    break;
                case 0x2: // 'slt' sets rd to 1 if rs1 < rs2, otherwise 0.
                    // Compare rs1 and rs2 as signed integers, set rd to 1 if rs1 is less than rs2, otherwise 0.
                    NEXT_STATE.REGS[decoded_rd] = (int32_t)CURRENT_STATE.REGS[decoded_rs1] < (int32_t)CURRENT_STATE.REGS[decoded_rs2];
                    break;
            }
            break;
        case 0x6F: // Handle UJ-type 'jal' instruction.
            {
                // Calculate the jump offset from the instruction, taking into account the sign bit and various parts of the instruction.
                int32_t offset = ((current_instruction & 0x80000000) >> 11) | // sign bit
                                 ((current_instruction & 0x7FE00000) >> 20) | // bits 10:1
                                 ((current_instruction & 0x00100000) >> 9)  | // bit 11
                                 ((current_instruction & 0x000FF000));        // bits 19:12
                NEXT_STATE.REGS[decoded_rd] = CURRENT_STATE.PC + 4; // Set the return address (PC + 4) in the destination register.
                NEXT_STATE.PC = CURRENT_STATE.PC + (offset << 1);  // Update the PC to the target address by adding the offset to the current PC.
            }
            break;
        case 0x63: // Handle SB-type 'bne' instruction.
            switch(decoded_funct3) {
                case 0x1: // 'bne' branches if registers are not equal.
                    {
                        // Calculate the branch offset, considering various parts of the instruction for sign extension and bit positions.
                        int32_t offset = ((current_instruction & 0x80000000) >> 19) | 
                                         ((current_instruction & 0x00000080) << 4) |
                                         ((current_instruction >> 20) & 0x7E0) | 
                                         ((current_instruction >> 7) & 0x1E);
                        // Perform the branch if rs1 and rs2 are not equal by updating the PC with the calculated offset.
                        if (CURRENT_STATE.REGS[decoded_rs1] != CURRENT_STATE.REGS[decoded_rs2]) {  
                            NEXT_STATE.PC = CURRENT_STATE.PC + offset;
                        }
                    }
                    break;
            }
            break;
    }
}

// Main function to process a single instruction cycle.
void process_instruction()
{
  /* execute one instruction here. You should use CURRENT_STATE and modify
   * values in NEXT_STATE. You can call mem_read_32() and mem_write_32() to
   * access memory. */
// Execute one instruction: fetch from memory, decode it, then execute.
  fetch(); // Fetch the next instruction from memory.
  decode(); // Decode the fetched instruction into its components.
  execute(); // Execute the decoded instruction.
}
