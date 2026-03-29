# Store [5,3,1,4,2] into memory
        ADDI $s0, $zero, 0
        ADDI $t0, $zero, 5
        SW   $t0, 0($s0)
        ADDI $t0, $zero, 3
        SW   $t0, 4($s0)
        ADDI $t0, $zero, 1
        SW   $t0, 8($s0)
        ADDI $t0, $zero, 4
        SW   $t0, 12($s0)
        ADDI $t0, $zero, 2
        SW   $t0, 16($s0)

# Bubble sort — fully unrolled, fixed offsets only

# Pass 1: compare (0,1),(1,2),(2,3),(3,4)
        LW   $t0, 0($s0)
        NOP
        LW   $t1, 4($s0)
        NOP
        SLT  $t2, $t1, $t0
        BEQ  $t2, $zero, s01
        NOP
        NOP
        SW   $t1, 0($s0)
        SW   $t0, 4($s0)
s01:
        LW   $t0, 4($s0)
        NOP
        LW   $t1, 8($s0)
        NOP
        SLT  $t2, $t1, $t0
        BEQ  $t2, $zero, s12
        NOP
        NOP
        SW   $t1, 4($s0)
        SW   $t0, 8($s0)
s12:
        LW   $t0, 8($s0)
        NOP
        LW   $t1, 12($s0)
        NOP
        SLT  $t2, $t1, $t0
        BEQ  $t2, $zero, s23
        NOP
        NOP
        SW   $t1, 8($s0)
        SW   $t0, 12($s0)
s23:
        LW   $t0, 12($s0)
        NOP
        LW   $t1, 16($s0)
        NOP
        SLT  $t2, $t1, $t0
        BEQ  $t2, $zero, s34
        NOP
        NOP
        SW   $t1, 12($s0)
        SW   $t0, 16($s0)
s34:

# Pass 2: compare (0,1),(1,2),(2,3)
        LW   $t0, 0($s0)
        NOP
        LW   $t1, 4($s0)
        NOP
        SLT  $t2, $t1, $t0
        BEQ  $t2, $zero, s201
        NOP
        NOP
        SW   $t1, 0($s0)
        SW   $t0, 4($s0)
s201:
        LW   $t0, 4($s0)
        NOP
        LW   $t1, 8($s0)
        NOP
        SLT  $t2, $t1, $t0
        BEQ  $t2, $zero, s212
        NOP
        NOP
        SW   $t1, 4($s0)
        SW   $t0, 8($s0)
s212:
        LW   $t0, 8($s0)
        NOP
        LW   $t1, 12($s0)
        NOP
        SLT  $t2, $t1, $t0
        BEQ  $t2, $zero, s223
        NOP
        NOP
        SW   $t1, 8($s0)
        SW   $t0, 12($s0)
s223:

# Pass 3: compare (0,1),(1,2)
        LW   $t0, 0($s0)
        NOP
        LW   $t1, 4($s0)
        NOP
        SLT  $t2, $t1, $t0
        BEQ  $t2, $zero, s301
        NOP
        NOP
        SW   $t1, 0($s0)
        SW   $t0, 4($s0)
s301:
        LW   $t0, 4($s0)
        NOP
        LW   $t1, 8($s0)
        NOP
        SLT  $t2, $t1, $t0
        BEQ  $t2, $zero, s312
        NOP
        NOP
        SW   $t1, 4($s0)
        SW   $t0, 8($s0)
s312:

# Pass 4: compare (0,1)
        LW   $t0, 0($s0)
        NOP
        LW   $t1, 4($s0)
        NOP
        SLT  $t2, $t1, $t0
        BEQ  $t2, $zero, s401
        NOP
        NOP
        SW   $t1, 0($s0)
        SW   $t0, 4($s0)
s401:

done:   J    done