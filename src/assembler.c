#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "isa.h"

/* ── Label table ──────────────────────────────────────── */
typedef struct { char name[64]; uint32_t word; } Label;
static Label  g_labels[MAX_LABELS];
static int    g_nlabels = 0;

/* ── Register name → number ───────────────────────────── */
static int reg_num(const char *s) {
    static const char *names[32] = {
        "$zero","$at","$v0","$v1","$a0","$a1","$a2","$a3",
        "$t0","$t1","$t2","$t3","$t4","$t5","$t6","$t7",
        "$s0","$s1","$s2","$s3","$s4","$s5","$s6","$s7",
        "$t8","$t9","$k0","$k1","$gp","$sp","$fp","$ra"
    };
    if (s[0] == '$' && isdigit((unsigned char)s[1])) return atoi(s + 1);
    for (int i = 0; i < 32; i++)
        if (strcmp(s, names[i]) == 0) return i;
    fprintf(stderr, "[asm] unknown reg: %s\n", s);
    return 0;
}

static uint32_t label_word(const char *name) {
    for (int i = 0; i < g_nlabels; i++)
        if (strcmp(g_labels[i].name, name) == 0) return g_labels[i].word;
    fprintf(stderr, "[asm] undefined label: %s\n", name);
    return 0;
}

/* ── Line utilities ───────────────────────────────────── */
static void strip_comment(char *s) {
    char *p = strpbrk(s, "#\n\r"); if (p) *p = '\0';
}
static char *ltrim(char *s) {
    while (*s && isspace((unsigned char)*s)) s++; return s;
}
static int tokenize(char *line, char *t[], int max) {
    int n = 0; char *p = line;
    while (*p && n < max) {
        while (*p == ' ' || *p == '\t' || *p == ',') p++;
        if (!*p) break;
        t[n++] = p;
        while (*p && *p != ' ' && *p != '\t' && *p != ',') p++;
        if (*p) *p++ = '\0';
    }
    return n;
}

/* Parse "offset(base)" into *off and *base */
static void parse_mem(char *s, int16_t *off, int *base) {
    char *lp = strchr(s, '(');
    if (!lp) { *off = 0; *base = 0; return; }
    *off = (int16_t)atoi(s);
    *lp++ = '\0';
    char *rp = strchr(lp, ')'); if (rp) *rp = '\0';
    *base = reg_num(lp);
}

/* ── Instruction encoder ──────────────────────────────── */
#define R(fn,d,s,t) ((uint32_t)((OP_RTYPE<<26)|((s)<<21)|((t)<<16)|((d)<<11)|(fn)))
#define I(op,s,t,i) ((uint32_t)(((op)<<26)|((s)<<21)|((t)<<16)|((uint16_t)(i))))
#define J(op,a)     ((uint32_t)(((op)<<26)|((a)&0x3FFFFFF)))
#define SHIFT_R(fn,d,t,sh) ((uint32_t)((OP_RTYPE<<26)|((0)<<21)|((t)<<16)|((d)<<11)|((sh)<<6)|(fn)))

static uint32_t encode(char *t[], int n, uint32_t cur_word) {
    if (n == 0) return NOP_INSTR;
    char *op = t[0];

    if (!strcasecmp(op,"ADD"))  return R(FUNCT_ADD,reg_num(t[1]),reg_num(t[2]),reg_num(t[3]));
    if (!strcasecmp(op,"SUB"))  return R(FUNCT_SUB,reg_num(t[1]),reg_num(t[2]),reg_num(t[3]));
    if (!strcasecmp(op,"AND"))  return R(FUNCT_AND,reg_num(t[1]),reg_num(t[2]),reg_num(t[3]));
    if (!strcasecmp(op,"OR"))   return R(FUNCT_OR, reg_num(t[1]),reg_num(t[2]),reg_num(t[3]));
    if (!strcasecmp(op,"SLT"))  return R(FUNCT_SLT,reg_num(t[1]),reg_num(t[2]),reg_num(t[3]));
    if (!strcasecmp(op,"SLL"))  return SHIFT_R(FUNCT_SLL,reg_num(t[1]),reg_num(t[2]),atoi(t[3]));
    if (!strcasecmp(op,"JR"))   return R(FUNCT_JR, 0,             reg_num(t[1]),0);

    if (!strcasecmp(op,"ADDI"))
        return I(OP_ADDI, reg_num(t[2]), reg_num(t[1]), atoi(t[3]));

    if (!strcasecmp(op,"LW")) {
        int16_t off; int base; parse_mem(t[2], &off, &base);
        return I(OP_LW, base, reg_num(t[1]), off);
    }
    if (!strcasecmp(op,"SW")) {
        int16_t off; int base; parse_mem(t[2], &off, &base);
        return I(OP_SW, base, reg_num(t[1]), off);
    }
    if (!strcasecmp(op,"BEQ")) {
        int16_t offset = (int16_t)((int32_t)label_word(t[3]) - (int32_t)(cur_word + 1));
        return I(OP_BEQ, reg_num(t[1]), reg_num(t[2]), offset);
    }
    if (!strcasecmp(op,"BNE")) {
        int16_t offset = (int16_t)((int32_t)label_word(t[3]) - (int32_t)(cur_word + 1));
        return I(OP_BNE, reg_num(t[1]), reg_num(t[2]), offset);
    }
    if (!strcasecmp(op,"J"))   return J(OP_J,   label_word(t[1]));
    if (!strcasecmp(op,"JAL")) return J(OP_JAL, label_word(t[1]));
    if (!strcasecmp(op,"NOP")) return NOP_INSTR;

    fprintf(stderr, "[asm] unknown op: %s\n", op);
    return NOP_INSTR;
}
#undef R
#undef I
#undef J

/* ── Two-pass assembler ───────────────────────────────── */
int assemble(const char *src_path, const char *out_path) {
    FILE *f = fopen(src_path, "r");
    if (!f) { perror(src_path); return -1; }

    /* Buffer all lines */
    static char raw[512][256];
    int nraw = 0;
    while (nraw < 512 && fgets(raw[nraw], 256, f)) {
        strip_comment(raw[nraw]);
        nraw++;
    }
    fclose(f);

    /* Pass 1: collect labels */
    g_nlabels = 0;
    uint32_t word = 0;
    for (int i = 0; i < nraw; i++) {
        char buf[256]; strncpy(buf, raw[i], 255);
        char *s = ltrim(buf);
        if (!*s) continue;
        char *colon = strchr(s, ':');
        if (colon) {
            *colon = '\0';
            strncpy(g_labels[g_nlabels].name, ltrim(s), 63);
            g_labels[g_nlabels++].word = word;
            s = ltrim(colon + 1);
        }
        if (*s) word++;
    }

    /* Pass 2: encode */
    static uint32_t instrs[512];
    int ni = 0;
    word = 0;
    for (int i = 0; i < nraw; i++) {
        char buf[256]; strncpy(buf, raw[i], 255);
        char *s = ltrim(buf);
        if (!*s) continue;
        char *colon = strchr(s, ':');
        if (colon) s = ltrim(colon + 1);
        if (!*s) continue;
        char *toks[16]; int nt = tokenize(s, toks, 16);
        if (nt == 0) continue;
        instrs[ni++] = encode(toks, nt, word++);
    }

    /* Write big-endian binary */
    FILE *out = fopen(out_path, "wb");
    if (!out) { perror(out_path); return -1; }

    uint32_t magic = PVM_MAGIC;
    uint32_t text_size = ni * 4; // Each instruction is 4 bytes

    // Write PVM_MAGIC (4 bytes)
    uint8_t magic_b[4] = {
        (magic>>24)&0xFF, (magic>>16)&0xFF,
        (magic>> 8)&0xFF, (magic>> 0)&0xFF
    };
    fwrite(magic_b, 1, 4, out);

    // Write text_size (4 bytes)
    uint8_t text_b[4] = {
        (text_size>>24)&0xFF, (text_size>>16)&0xFF,
        (text_size>> 8)&0xFF, (text_size>> 0)&0xFF
    };
    fwrite(text_b, 1, 4, out);

    // Skip data_size for now, as vm.c expects magic and text_size as 8-byte header.
    // uint32_t data_size = 0;
    // uint8_t data_b[4] = {
    //     (data_size>>24)&0xFF, (data_size>>16)&0xFF,
    //     (data_size>> 8)&0xFF, (data_size>> 0)&0xFF
    // };
    // fwrite(data_b, 1, 4, out);

    for (int i = 0; i < ni; i++) {
        uint8_t b[4] = {
            (instrs[i]>>24)&0xFF, (instrs[i]>>16)&0xFF,
            (instrs[i]>> 8)&0xFF, (instrs[i]>> 0)&0xFF
        };
        fwrite(b, 1, 4, out);
    }
    fclose(out);
    printf("[asm] %s -> %s  (%d instructions, text_size=%u)\n", src_path, out_path, ni, text_size);
    return ni;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input.asm> <output.pvm>\n", argv[0]);
        return 1;
    }

    if (assemble(argv[1], argv[2]) == -1) {
        fprintf(stderr, "Assembly failed.\n");
        return 1;
    }

    return 0;
}
