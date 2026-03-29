#ifndef VM_H
#define VM_H

#include <stdint.h>
#include <stdbool.h>
#include "isa.h"
#include "pipeline.h"

typedef struct {
    uint32_t pc;                    /* word-addressed program counter */
    int32_t  regs[NUM_REGS];
    uint8_t  dmem[DMEM_BYTES];
    uint32_t imem[IMEM_WORDS];
    uint32_t n_instrs;
    Pipeline pipeline;
    uint64_t cycle;
    bool     halted;
    uint32_t dmem_size;
    bool     step_mode;
} VM;

/* Lifecycle */
void vm_init(VM *vm);
bool vm_load_program(VM *vm, const char *pvm_path);
void vm_run(VM *vm);
void vm_clock(VM *vm);
void vm_cleanup(VM *vm);

/* Memory ops (memory.c) */
int32_t reg_read(const VM *vm, uint8_t r);
void    reg_write(VM *vm, uint8_t r, int32_t v);
int32_t dmem_lw(const VM *vm, uint32_t byte_addr);
void    dmem_sw(VM *vm, uint32_t byte_addr, int32_t v);
void    mem_reset(VM *vm);

#endif /* VM_H */
