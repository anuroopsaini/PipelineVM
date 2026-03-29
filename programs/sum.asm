# Sum of first 10 natural numbers
# Result will be in $t0

        ADDI    $t0, $zero, 0      # sum = 0
        ADDI    $t1, $zero, 1      # counter i = 1
        ADDI    $t2, $zero, 10     # N = 10 (loop counter)

sum_loop:
        BEQ     $t2, $zero, sum_done  # if N == 0, exit loop

        ADD     $t0, $t0, $t1      # sum += i
        ADDI    $t1, $t1, 1        # i++
        ADDI    $t2, $t2, -1       # N--
        J       sum_loop            # jump back to loop

sum_done:
        # Halt the program
        # Your ISA may require using a register check or just stopping
        # For demo purposes, we can leave the last instruction as NOP
        sum_done: J sum_done