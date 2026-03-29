#include <stdio.h>
#include <stdlib.h>
#include "isa.h"
#include "vm.h"
#include "viz.h"

#define CLR   "\x1b[2J\x1b[H"
#define RST   "\x1b[0m"
#define BOLD  "\x1b[1m"
#define DIM   "\x1b[2m"
#define RED   "\x1b[31m"
#define GRN   "\x1b[32m"
#define YEL   "\x1b[33m"
#define BLU   "\x1b[34m"
#define MAG   "\x1b[35m"
#define CYN   "\x1b[36m"
#define WHT   "\x1b[37m"
#define BGBLK "\x1b[40m"

void viz_render(const VM *vm) {
    const Pipeline *p = &vm->pipeline;
    printf(CLR);

    /* ── Header ── */
    printf(BOLD CYN
        "╔══════════════════════════════════════════════════════════╗\n"
        "║          PipelineVM  —  MIPS 5-Stage Pipeline Sim       ║\n"
        "╠══════════════════════════════════════════════════════════╣\n" RST);
    printf(CYN "║" RST "  Cycle: " BOLD YEL "%-10llu" RST
               "  PC: " BOLD YEL "0x%04X" RST
               "  Instructions: " BOLD YEL "%-6u" RST
               CYN "      ║\n" RST,
        (unsigned long long)vm->cycle, vm->pc, vm->n_instrs);
    printf(CYN "╚══════════════════════════════════════════════════════════╝\n" RST);

    /* ── Status banner ── */
    if (p->stall)
        printf(YEL "  ⚠  STALL — load-use hazard detected (bubble inserted)\n" RST);
    else if (p->branch_flush)
        printf(MAG "  ↺  BRANCH TAKEN — flushing 2 pipeline stages\n" RST);
    else if (p->data_hazard)
        printf(CYN "  →  FORWARDING — data hazard resolved without stall\n" RST);
    else
        printf(DIM "  ·  No hazards this cycle\n" RST);

    /* ── Pipeline stages ── */
    printf("\n");
    printf("  ┌──────────────┬──────────────┬──────────────┬──────────────┬──────────────┐\n");
    printf("  │" BOLD CYN "      IF      " RST
             "│" BOLD GRN "      ID      " RST
             "│" BOLD YEL "      EX      " RST
             "│" BOLD MAG "     MEM      " RST
             "│" BOLD BLU "      WB      " RST "│\n");
    printf("  ├──────────────┼──────────────┼──────────────┼──────────────┼──────────────┤\n");

    const char *stages[5] = {
        p->if_id.valid  ? p->if_id.mnem  : "  ────────  ",
        p->id_ex.valid  ? p->id_ex.mnem  : "  ────────  ",
        p->ex_mem.valid ? p->ex_mem.mnem : "  ────────  ",
        p->mem_wb.valid ? p->mem_wb.mnem : "  ────────  ",
        p->wb_valid     ? p->wb_mnem     : "  ────────  "
    };
    const char *colors[5] = { CYN, GRN, YEL, MAG, BLU };
    printf("  │");
    for (int i = 0; i < 5; i++)
        printf(" %s%-12.12s" RST " │", colors[i], stages[i]);
    printf("\n");
    printf("  └──────────────┴──────────────┴──────────────┴──────────────┴──────────────┘\n");

    /* ── Registers ── */
    printf("\n" BOLD "  REGISTERS\n" RST);
    static const char *rnames[32] = {
        "$0", "$at","$v0","$v1","$a0","$a1","$a2","$a3",
        "$t0","$t1","$t2","$t3","$t4","$t5","$t6","$t7",
        "$s0","$s1","$s2","$s3","$s4","$s5","$s6","$s7",
        "$t8","$t9","$k0","$k1","$gp","$sp","$fp","$ra"
    };
    int col = 0;
    for (int i = 0; i < 32; i++) {
        if (vm->regs[i] != 0)
            printf("  " BOLD "%-4s" RST "=" GRN "%-10d" RST, rnames[i], vm->regs[i]);
        else
            printf("  " DIM "%-4s=%-10d" RST, rnames[i], 0);
        if (++col % 4 == 0) printf("\n");
    }
    if (col % 4 != 0) printf("\n");

    /* ── Data memory ── */
    printf("\n" BOLD "  DATA MEMORY\n" RST);
    int nonzero_max = 0;
    for (int i = 0; i < (int)(DMEM_BYTES/4); i++) {
        uint32_t w = ((uint32_t)vm->dmem[i*4]<<24)|((uint32_t)vm->dmem[i*4+1]<<16)
                    |((uint32_t)vm->dmem[i*4+2]<<8)|vm->dmem[i*4+3];
        if (w != 0) nonzero_max = i;
    }
    int show = nonzero_max + 4;
    if (show < 8) show = 8;
    if (show > 32) show = 32;
    for (int i = 0; i < show; i++) {
        uint32_t w = ((uint32_t)vm->dmem[i*4]<<24)|((uint32_t)vm->dmem[i*4+1]<<16)
                    |((uint32_t)vm->dmem[i*4+2]<<8)|vm->dmem[i*4+3];
        printf("  " DIM "[%03x] " RST, i * 4);
        if (w != 0) printf(GRN "%08x" RST, w);
        else        printf(DIM "%08x" RST, w);
        if ((i+1) % 4 == 0) printf("\n");
        else                 printf("  ");
    }
    printf("\n");

    /* ── Controls ── */
    printf(DIM "  [ENTER] step    [r] run    [q] quit\n" RST);
    fflush(stdout);
}