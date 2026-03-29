# Fibonacci — computes F(10) = 55
# Result in $t0 when "done" label is reached.
# $t0 = current,  $t1 = previous,  $t2 = counter,  $t3 = temp

        ADDI $t0, $zero, 1      # F(1) = 1
        ADDI $t1, $zero, 0      # F(0) = 0
        ADDI $t2, $zero, 9      # iterate 9 more times

loop:   ADD  $t3, $t0, $t1     # t3 = F(n) + F(n-1)
        ADD  $t1, $t0, $zero   # t1 = old F(n)
        ADD  $t0, $t3, $zero   # t0 = new F(n+1)
        ADDI $t2, $t2, -1      # counter--
        BEQ  $t2, $zero, done  # if counter == 0, done
        J    loop

done:   J    done               # halt — result: $t0 = 55