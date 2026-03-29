#include "vm.h"
#include <string.h>
#include <stdio.h>

int32_t reg_read(const VM *vm, uint8_t r) {
    return (r == 0) ? 0 : vm->regs[r];
}

void reg_write(VM *vm, uint8_t r, int32_t v) {
    if (r != 0) vm->regs[r] = v;
}

int32_t dmem_lw(const VM *vm, uint32_t addr) {
    if (addr + 3 >= DMEM_BYTES) {
        fprintf(stderr, "[mem] LW OOB: 0x%04X\n", addr);
        return 0;
    }
    return (int32_t)(
        ((uint32_t)vm->dmem[addr  ] << 24) |
        ((uint32_t)vm->dmem[addr+1] << 16) |
        ((uint32_t)vm->dmem[addr+2] <<  8) |
        ((uint32_t)vm->dmem[addr+3])
    );
}

void dmem_sw(VM *vm, uint32_t addr, int32_t v) {
    if (addr + 3 >= DMEM_BYTES) {
        fprintf(stderr, "[mem] SW OOB: 0x%04X\n", addr);
        return;
    }
    vm->dmem[addr  ] = (v >> 24) & 0xFF;
    vm->dmem[addr+1] = (v >> 16) & 0xFF;
    vm->dmem[addr+2] = (v >>  8) & 0xFF;
    vm->dmem[addr+3] = (v      ) & 0xFF;
}

void mem_reset(VM *vm) {
    memset(vm->dmem, 0, DMEM_BYTES);
    memset(vm->regs, 0, sizeof(vm->regs));
}