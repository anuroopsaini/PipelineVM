#include <stdio.h>
#include "isa.h"

// Helper function to get the string name of an instruction
const char* get_instruction_name(DecodedInstruction* instr) {
    if (!instr->is_valid) {
        return "NOP"; // Or "INVALID"
    }

    // Handle R-type instructions (opcode 0x00)
    if (instr->opcode == OP_RTYPE) { // All R-type have opcode 0x00 (OP_RTYPE is just a placeholder for R-type opcode)
        switch (instr->funct) {
            case FUNCT_ADD: return "ADD";
            case FUNCT_ADDU: return "ADDU";
            case FUNCT_SUB: return "SUB";
            case FUNCT_AND: return "AND";
            case FUNCT_OR:  return "OR";
            case FUNCT_SLT: return "SLT";
            case FUNCT_JR:  return "JR";
            case FUNCT_SYSCALL: return "SYSCALL";
            case FUNCT_SLL: return "SLL";
            default: return "R-TYPE UNKNOWN";
        }
    }

    // Handle I-type instructions
    switch (instr->opcode) {
        case OP_ADDI: return "ADDI";
        case OP_LW:   return "LW";
        case OP_SW:   return "SW";
        case OP_BEQ:  return "BEQ";
        case OP_BNE:  return "BNE";
        case OP_BLEZ: return "BLEZ";
        case OP_BGTZ: return "BGTZ";
        case OP_LUI:  return "LUI";
        case OP_ORI:  return "ORI";
        case OP_XORI: return "XORI";
        default: break; // Fall through to J-type or unknown
    }

    // Handle J-type instructions
    switch (instr->opcode) {
        case OP_J:    return "J";
        case OP_JAL:  return "JAL";
        default: return "UNKNOWN";
    }
}
