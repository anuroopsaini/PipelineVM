#include <stdio.h>
#include <stdlib.h>
#include "isa.h"
#include "vm.h"
#include "viz.h"

// Placeholder for visualization logic
void viz_render(VM* vm) {
    // Clear screen and print pipeline state using ANSI colors
    printf("\033[H\033[J"); // Clear screen
    printf("\n═══════════════════════════════════════════════\n");
    printf("  PipelineVM  |  PC: 0x%04x \n", vm->pipeline.if_id.valid ? vm->pipeline.if_id.pc4 - 1 : vm->pc);
    printf("═══════════════════════════════════════════════\n");
    fflush(stdout);

    printf("  IF         |  ID         |  EX         |  MEM        |  WB         \n");
    printf("-------------|-------------|-------------|-------------|------------- \n");

    // Get instruction names for each stage
    const char* if_id_instr = vm->pipeline.if_id.valid ? instr_mnem(vm->pipeline.if_id.instr) : "NOP";
    const char* id_ex_instr = vm->pipeline.id_ex.valid ? vm->pipeline.id_ex.mnem : "NOP";
    const char* ex_mem_instr = vm->pipeline.ex_mem.valid ? vm->pipeline.ex_mem.mnem : "NOP";
    const char* mem_wb_instr = vm->pipeline.mem_wb.valid ? vm->pipeline.mem_wb.mnem : "NOP";

    printf("  %-10s |  %-10s |  %-10s |  %-10s |  %-10s \n",
           if_id_instr, id_ex_instr, ex_mem_instr, mem_wb_instr, "N/A"); // WB doesn't have a next stage
    printf("-------------|-------------|-------------|-------------|------------- \n");

    printf("\n  Registers:\n");
    for (int i = 0; i < NUM_REGS; ++i) {
        printf("  R%-2d:0x%08x%s", i, vm->regs[i], (i + 1) % 4 == 0 ? "\n" : "  ");
    }
    printf("\n");

    // Display a portion of memory (e.g., around PC or data segment)
    printf("\n  Instruction Memory [0x%04x-0x%04x]:\n", 0, (vm->n_instrs > 0 ? (vm->n_instrs -1) * 4 : 0)); // Display all loaded instructions
    int display_limit = (vm->n_instrs < 16) ? vm->n_instrs : 16; // Display up to 16 instructions
    for (int i = 0; i < display_limit; ++i) {
        printf("%08x%s", vm->imem[i], (i + 1) % 4 == 0 ? "\n" : "  ");
    }
    // If fewer than 16 instructions, fill the rest with XXXXXXXX
    for (int i = display_limit; i < 16; ++i) {
        printf("XXXXXXXX%s", (i + 1) % 4 == 0 ? "\n" : "  ");
    }
    printf("\n");

    // Display Data Memory
    printf("\n  Data Memory [0x%04x-0x%04x]:\n", 0, 15*4); // Display first 16 words of data memory
    for (int i = 0; i < 16; ++i) {
        printf("%08x%s", vm->dmem[i], (i + 1) % 4 == 0 ? "\n" : "  ");
    }
    printf("\n");
    printf("═══════════════════════════════════════════════\n");
}
