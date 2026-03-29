#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#define PVM_MAGIC 0xDEADC0DE
#include "vm.h"
#include "viz.h"

/* ── Mnemonic builder ─────────────────────────────────── */
const char *instr_mnem(uint32_t instr) {
    static char buf[24];
    if (instr == NOP_INSTR) { snprintf(buf,24,"NOP"); return buf; }
    uint8_t op = OPCODE(instr), fn = FUNCT(instr);
    uint8_t d=RD(instr), s=RS(instr), t=RT(instr);
    int16_t im = IMM(instr);
    switch (op) {
        case OP_RTYPE:
            switch (fn) {
                case FUNCT_ADD: snprintf(buf,24,"ADD $%d,$%d,$%d",d,s,t); break;
                case FUNCT_SUB: snprintf(buf,24,"SUB $%d,$%d,$%d",d,s,t); break;
                case FUNCT_AND: snprintf(buf,24,"AND $%d,$%d,$%d",d,s,t); break;
                case FUNCT_OR:  snprintf(buf,24,"OR  $%d,$%d,$%d",d,s,t); break;
                case FUNCT_SLT: snprintf(buf,24,"SLT $%d,$%d,$%d",d,s,t); break;
                case FUNCT_JR:  snprintf(buf,24,"JR  $%d",s); break;
                case FUNCT_SLL: snprintf(buf,24,"SLL $%d,$%d,%d",d,t,(instr>>6)&0x1F); break;
                default:        snprintf(buf,24,"R?? fn=%02x",fn); break;
            }
            break;
        case OP_ADDI: snprintf(buf,24,"ADDI $%d,%d",t,im); break;
        case OP_LW:   snprintf(buf,24,"LW $%d,%d($%d)",t,im,s); break;
        case OP_SW:   snprintf(buf,24,"SW $%d,%d($%d)",t,im,s); break;
        case OP_BEQ:  snprintf(buf,24,"BEQ $%d,$%d",s,t); break;
        case OP_BNE:  snprintf(buf,24,"BNE $%d,$%d",s,t); break;
        case OP_J:    snprintf(buf,24,"J   %u",JADDR(instr)); break;
        case OP_JAL:  snprintf(buf,24,"JAL %u",JADDR(instr)); break;
        default:      snprintf(buf,24,"??? %08x",instr); break;
    }
    return buf;
}

/* ── Control unit ─────────────────────────────────────── */
static Ctrl decode_ctrl(uint8_t op) {
    Ctrl c = {0};
    switch (op) {
        case OP_RTYPE: c.reg_dst=1; c.reg_write=1; c.alu_op=3; break;
        case OP_ADDI:  c.alu_src=1; c.reg_write=1; c.alu_op=2; break;
        case OP_LW:    c.alu_src=1; c.reg_write=1; c.mem_read=1;
                       c.mem_to_reg=1; c.alu_op=2; break;
        case OP_SW:    c.alu_src=1; c.mem_write=1; c.alu_op=2; break;
        case OP_BEQ:   c.branch=1; c.alu_op=6; break;
        case OP_BNE:   c.branch=1; c.branch_ne=1; c.alu_op=6; break;
        case OP_J:     c.jump=1; break;
        case OP_JAL:   c.jump=1; c.jal=1; c.reg_write=1; break;
        default: break;
    }
    return c;
}

/* ── ALU ──────────────────────────────────────────────── */
static uint8_t alu_ctrl(uint8_t alu_op, uint8_t funct) {
    if (alu_op != 3) return alu_op;
    switch (funct) {
        case FUNCT_ADD: return 2;
        case FUNCT_SUB: return 6;
        case FUNCT_AND: return 0;
        case FUNCT_OR:  return 1;
        case FUNCT_SLT: return 7;
        case FUNCT_SLL: return 8;
        default:        return 2;
    }
}

static int32_t do_alu(uint8_t op, int32_t a, int32_t b, bool *z) {
    int32_t r;
    switch (op) {
        case 0: r = a & b; break;
        case 1: r = a | b; break;
        case 2: r = a + b; break;
        case 6: r = a - b; break;
        case 7: r = (a < b) ? 1 : 0; break;
        case 8: r = b << (int)(a & 0x1F); break;
        default: r = a + b; break;
    }
    if (z) *z = (r == 0);
    return r;
}

/* ── Hazard detection ─────────────────────────────────── */
static bool load_use_hazard(const IDEX *s1, const IFID *s0) {
    if (!s1->valid || !s0->valid || !s1->ctrl.mem_read) return false;
    uint8_t dest = s1->rt; /* LW destination is rt */
    if (dest == 0) return false;
    uint8_t rs = RS(s0->instr);
    uint8_t rt = RT(s0->instr);
    return (rs == dest || rt == dest);
}

/* ── VM lifecycle ─────────────────────────────────────── */
void vm_init(VM *vm) {
    memset(vm, 0, sizeof(VM));
    mem_reset(vm);
    vm->step_mode = false;
}

void vm_cleanup(VM *vm) {
    (void)vm;
}

bool vm_load_program(VM *vm, const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) { perror(path); return false; }

    uint32_t magic, text_size_bytes;
    if (fread(&magic, 4, 1, f) != 1 || fread(&text_size_bytes, 4, 1, f) != 1) {
        perror("Error reading PVM header");
        fclose(f);
        return false;
    }

    /* Byte-swap big-endian header to host */
    magic = ((magic>>24)&0xFF) | ((magic>>8)&0xFF00) |
            ((magic<<8)&0xFF0000) | ((magic<<24)&0xFF000000);
    text_size_bytes = ((text_size_bytes>>24)&0xFF) | ((text_size_bytes>>8)&0xFF00) |
                      ((text_size_bytes<<8)&0xFF0000) | ((text_size_bytes<<24)&0xFF000000);

    if (magic != PVM_MAGIC) {
        fprintf(stderr, "Invalid PVM magic: 0x%08x\n", magic);
        fclose(f);
        return false;
    }

    if (text_size_bytes == 0 || text_size_bytes > IMEM_BYTES) {
        fprintf(stderr, "Invalid text size: %u\n", text_size_bytes);
        fclose(f);
        return false;
    }

    vm->n_instrs = text_size_bytes / 4;
    uint8_t buf[IMEM_BYTES];

    if (fread(buf, 1, text_size_bytes, f) != text_size_bytes) {
        perror("Error reading instructions");
        fclose(f);
        return false;
    }
    fclose(f);

    for (uint32_t i = 0; i < vm->n_instrs; i++)
        vm->imem[i] = ((uint32_t)buf[i*4  ] << 24)
                    | ((uint32_t)buf[i*4+1] << 16)
                    | ((uint32_t)buf[i*4+2] <<  8)
                    | ((uint32_t)buf[i*4+3]);

    return true;
}

/* ── One clock cycle ──────────────────────────────────── */
void vm_clock(VM *vm) {
    IFID  s0 = vm->pipeline.if_id;
    IDEX  s1 = vm->pipeline.id_ex;
    EXMEM s2 = vm->pipeline.ex_mem;
    MEMWB s3 = vm->pipeline.mem_wb;

    Pipeline *p = &vm->pipeline;
    p->stall        = false;
    p->data_hazard  = false;
    p->branch_flush = false;
    p->wb_valid     = false;

    /* ── WB ─────────────────────────────────────────────── */
    if (s3.valid && s3.ctrl.reg_write && s3.dest != 0) {
        int32_t wd = s3.ctrl.mem_to_reg ? s3.mem_data : s3.alu_result;
        reg_write(vm, s3.dest, wd);
        strncpy(p->wb_mnem, s3.mnem, 23);
        p->wb_valid = true;
    }

    /* ── MEM ────────────────────────────────────────────── */
    {
        MEMWB *out = &p->mem_wb;
        memset(out, 0, sizeof(MEMWB));
        if (s2.valid) {
            out->valid      = true;
            out->alu_result = s2.alu_result;
            out->dest       = s2.dest;
            out->ctrl       = s2.ctrl;
            strncpy(out->mnem, s2.mnem, 23);
            if (s2.ctrl.mem_read) {
                out->mem_data = dmem_lw(vm, (uint32_t)s2.alu_result);
            }
            if (s2.ctrl.mem_write) {
                dmem_sw(vm, (uint32_t)s2.alu_result, s2.rt_val);
            }
        }
    }

    /* ── Stall (load-use hazard) ────────────────────────── */
    if (load_use_hazard(&s1, &s0)) {
        p->stall      = true;
        p->data_hazard = true;
        p->if_id = s0;
        memset(&p->id_ex,  0, sizeof(IDEX));
        memset(&p->ex_mem, 0, sizeof(EXMEM));
        vm->cycle++;
        return;
    }

    /* ── EX ─────────────────────────────────────────────── */
    uint32_t new_pc = vm->pc;
    bool     flush2 = false;
    {
        EXMEM *out = &p->ex_mem;
        memset(out, 0, sizeof(EXMEM));
        if (s1.valid) {
            out->valid = true;
            out->ctrl  = s1.ctrl;
            strncpy(out->mnem, s1.mnem, 23);

            /* Forwarding: EX/MEM → EX */
            int32_t rs_v = s1.rs_val, rt_v = s1.rt_val;
            if (s2.valid && s2.ctrl.reg_write && s2.dest != 0) {
                if (s2.dest == s1.rs) { rs_v = s2.alu_result; p->data_hazard = true; }
                if (s2.dest == s1.rt) { rt_v = s2.alu_result; p->data_hazard = true; }
            }
            /* Forwarding: MEM/WB → EX */
            if (s3.valid && s3.ctrl.reg_write && s3.dest != 0) {
                int32_t wv = s3.ctrl.mem_to_reg ? s3.mem_data : s3.alu_result;
                bool ex_rs = s2.valid && s2.ctrl.reg_write && s2.dest == s1.rs;
                bool ex_rt = s2.valid && s2.ctrl.reg_write && s2.dest == s1.rt;
                if (s3.dest == s1.rs && !ex_rs) { rs_v = wv; p->data_hazard = true; }
                if (s3.dest == s1.rt && !ex_rt) { rt_v = wv; p->data_hazard = true; }
            }

            out->rt_val = rt_v; /* for SW */

            /* ALU */
            int32_t alu_b = s1.ctrl.alu_src ? (int32_t)s1.imm : rt_v;
            uint8_t ac    = alu_ctrl(s1.ctrl.alu_op, s1.funct);
            bool zero;
            int32_t alu_a = (s1.funct == FUNCT_SLL) ? (int32_t)s1.shamt : rs_v;
            out->alu_result = do_alu(ac, alu_a, alu_b, &zero);
            out->zero       = zero;
            out->dest       = s1.ctrl.reg_dst ? s1.rd : s1.rt;
            out->branch_tgt = s1.pc4 + (uint32_t)((int32_t)s1.imm);

            /* Branch resolution */
            if (s1.ctrl.branch) {
                bool taken = s1.ctrl.branch_ne ? !zero : zero;
                if (taken) {
                    new_pc          = out->branch_tgt;
                    flush2          = true;
                    p->branch_flush = true;
                }
            }
        }
    }

    /* ── ID ─────────────────────────────────────────────── */
    bool flush1 = false;
    {
        IDEX *out = &p->id_ex;
        memset(out, 0, sizeof(IDEX));
        if (s0.valid && !flush2) {
            uint32_t ins = s0.instr;
            uint8_t  op  = OPCODE(ins);
            uint8_t  rs  = RS(ins), rt = RT(ins), rd = RD(ins);
            uint8_t  fn  = FUNCT(ins);

            out->valid  = true;
            out->pc4    = s0.pc4;
            out->rs     = rs; out->rt = rt; out->rd = rd;
            out->funct  = fn;
            out->shamt  = (ins >> 6) & 0x1F;
            out->imm    = IMM(ins);
            out->rs_val = reg_read(vm, rs);
            out->rt_val = reg_read(vm, rt);
            out->ctrl   = decode_ctrl(op);
            strncpy(out->mnem, s0.mnem, 23);

            if (op == OP_J || op == OP_JAL) {
                if (op == OP_JAL)
                    reg_write(vm, 31, (int32_t)s0.pc4);
                new_pc  = JADDR(ins);
                flush1  = true;
                out->valid = false;
            }
            if (op == OP_RTYPE && fn == FUNCT_JR) {
                new_pc  = (uint32_t)reg_read(vm, rs);
                flush1  = true;
                out->valid = false;
            }
        }
    }

    /* ── IF ─────────────────────────────────────────────── */
    {
        IFID *out = &p->if_id;
        if (flush2) {
            memset(out,       0, sizeof(IFID));
            memset(&p->id_ex, 0, sizeof(IDEX));
            vm->pc = new_pc;
        } else if (flush1) {
            memset(out, 0, sizeof(IFID));
            vm->pc = new_pc;
        } else {
            memset(out, 0, sizeof(IFID));
            if (vm->pc < vm->n_instrs) {
                uint32_t ins = vm->imem[vm->pc];
                out->instr = ins;
                out->pc4   = vm->pc + 1;
                out->valid = true;
                strncpy(out->mnem, instr_mnem(ins), 23);

                if (OPCODE(ins) == OP_J && JADDR(ins) == vm->pc)
                    vm->halted = true;

                vm->pc++;
            } else {
                if (!s0.valid && !s1.valid && !s2.valid && !s3.valid)
                    vm->halted = true;
            }
        }
    }

    vm->cycle++;
}

/* ── Run loop ─────────────────────────────────────────── */
void vm_run(VM *vm) {
    struct termios old, raw;
    tcgetattr(STDIN_FILENO, &old);
    raw = old;
    raw.c_lflag &= (tcflag_t)~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);

    while (!vm->halted) {
        vm_clock(vm);
        viz_render(vm);

        if (vm->step_mode) {
            int c = getchar();
            if (c == 'q' || c == 'Q') break;
            if (c == 'r' || c == 'R') vm->step_mode = false;
        } else {
            usleep(120000);
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &old);
    printf("\n\nProgram complete. Cycles: %llu\n",
           (unsigned long long)vm->cycle);
}