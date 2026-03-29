start:  ADDI $t0, $zero, 10    ; $t0 = 10
        ADDI $t1, $zero, 20    ; $t1 = 20
        ADD $t2, $t0, $t1     ; $t2 = $t0 + $t1
        J start         ; Loop back to start (for testing labels)