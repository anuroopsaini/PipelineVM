#ifndef ISA_H
#define ISA_H

#include <stdint.h>
#include <stdbool.h>

#define PVM_MAGIC 0xDEADC0DE
#define NOP_INSTR 0x00000000

#define IMEM_WORDS 256
#define IMEM_BYTES (IMEM_WORDS * 4)
#define DMEM_BYTES 1024
#define MAX_LABELS 256

// Instruction types and fields
#define OPCODE(instr) ((instr >> 26) & 0x3F)
#define RS(instr)     ((instr >> 21) & 0x1F)
#define RT(instr)     ((instr >> 16) & 0x1F)
#define RD(instr)     ((instr >> 11) & 0x1F)
#define SHAMT(instr)  ((instr >> 6)  & 0x1F)
#define FUNCT(instr)  (instr & 0x3F)
#define IMM(instr)    ((int16_t)(instr & 0xFFFF))
#define IMM_S(instr)  ((int16_t)(instr & 0xFFFF)) // Signed immediate
#define JADDR(instr)  (instr & 0x3FFFFFF)

// Opcodes
#define OP_RTYPE    0x00
#define OP_J        0x02
#define OP_JAL      0x03
#define OP_BEQ      0x04
#define OP_BNE      0x05
#define OP_BLEZ     0x06
#define OP_BGTZ     0x07
#define OP_ADDI     0x08
#define OP_ORI      0x0D
#define OP_XORI     0x0E
#define OP_LUI      0x0F
#define OP_LW       0x23
#define OP_SW       0x2B
#define OP_SYSCALL  0x0C // This one was already there

// Funct codes (for R-type instructions)
#define FUNCT_SLL   0x00
#define FUNCT_NOP   0x00
#define FUNCT_SRL   0x02
#define FUNCT_SRA   0x03
#define FUNCT_JR    0x08
#define FUNCT_SYSCALL 0x0C
#define FUNCT_ADD   0x20
#define FUNCT_ADDU  0x21
#define FUNCT_SUB   0x22
#define FUNCT_AND   0x24
#define FUNCT_OR    0x25
#define FUNCT_SLT   0x2A

typedef struct {
    bool is_valid;
    uint32_t raw_instr;
    uint8_t opcode;
    uint8_t funct;
    uint8_t rs;
    uint8_t rt;
    uint8_t rd;
    int16_t imm;
    uint32_t target_addr;
} DecodedInstruction;

#define REG_SP 29
#define REG_RA 31
#define REG_V0 2
#define REG_A0 4
#define NUM_REGS 32

const char *instr_mnem(uint32_t instr);

#endif // ISA_H
