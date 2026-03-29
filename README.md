# PipelineVM
*A MIPS-Inspired Bytecode Virtual Machine in C*

---

## 🖥️ Project Overview

**PipelineVM** is a fully functional virtual machine written in C that simulates a simplified MIPS-like instruction set with a **5-stage pipeline**.

You write assembly-like programs, and an included assembler compiles them into bytecode, which the VM executes — **showing in real-time which instruction is in IF, ID, EX, MEM, and WB at every clock cycle**.

This project serves as a **systems programming exercise, an educational teaching tool**, and a **direct extension of CMPEN 331 coursework**.

---

## Architecture

```text
[ .asm file ]
     │
     ▼
┌──────────┐
│ Assembler │  (assembler.c)
└──────────┘
     │ bytecode (.pvm file)
     ▼
┌──────────────────────────────┐
│         PipelineVM           │
│                              │
│  ┌─────────────────────────┐ │
│  │   5-Stage Pipeline Sim  │ │
│  │  IF → ID → EX → MEM→WB │ │
│  └─────────────────────────┘ │
│  ┌──────────┐  ┌──────────┐  │
│  │ Register │  │  Memory  │  │
│  │  File    │  │  (4KB)   │  │
│  │ (32 regs)│  │  array   │  │
│  └──────────┘  └──────────┘  │
└──────────────────────────────┘
     │
     ▼
[ Terminal Visualizer ]  (viz.c)
```

### **Option 2: Use Markdown lists**
```
## Supported ISA

**R-type:**
- ADD rd, rs, rt
- SUB rd, rs, rt
- AND rd, rs, rt
- OR rd, rs, rt
- SLT rd, rs, rt (set less than)

**I-type:**
- ADDI rd, rs, imm
- LW rd, offset(rs)
- SW rs, offset(rd)
- BEQ rs, rt, label

**J-type:**
- J label
- JAL label
- JR rs
```


## File Structure
```
pipelinevm/
├── src/
│   ├── assembler.c     # Tokenizer + encoder → .pvm bytecode
│   ├── vm.c            # Main fetch-decode-execute loop
│   ├── pipeline.c      # Pipeline stage structs + stall/hazard logic
│   ├── memory.c        # Register file + data memory abstraction
│   ├── viz.c           # Terminal visualization (ANSI colors)
│   └── main.c
├── programs/
│   ├── fib.asm         # Fibonacci demo
│   ├── sort.asm        # Bubble sort demo
│   └── hello.asm       # Simple loop
├── include/
│   ├── isa.h           # Opcode enum, instruction struct
│   ├── pipeline.h
│   └── vm.h
└── Makefile
```


## Terminal Visualizer
Every clock cycle, the terminal prints the current pipeline state:
```
═══════════════════════════════════════════════
  PipelineVM  |  Cycle: 14  |  PC: 0x001C
═══════════════════════════════════════════════

  IF  │  ID  │  EX  │ MEM  │  WB
──────┼───────┼───────┼──────┼───────
 BEQ  │ ADD  │ LW   │ ADDI │ SUB
 r2r3 │r1,r2 │r3,4  │r1,5  │r4,r5
──────┼───────┼───────┼──────┼───────
                ⚠ DATA HAZARD (stall)

  Registers:
  $0=0  $1=13  $2=8  $3=5  $4=3  $5=2 ...

  Memory [0x00–0x10]:
  00 00 00 0D 00 00 00 08 ...
═══════════════════════════════════════════════
  [SPACE] step  |  [R] run  |  [Q] quit
```



## License
MIT License — feel free to use, modify, and share.
