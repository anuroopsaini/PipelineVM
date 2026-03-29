# Countdown: $t0 goes from 10 down to 0
        ADDI $t0, $zero, 10

loop:   ADDI $t0, $t0, -1
        BEQ  $t0, $zero, done
        J    loop

done:   J    done