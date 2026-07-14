# Builds a 5x5 multiplication table (table[i][j] = (i+1)*(j+1)) into memory,
# demonstrating RV32M's mul alongside nested loops and 2D index arithmetic.
.globl _start
_start:
    li   t0, 0x4000      # base address of the table
    li   s0, 0           # i = 0
outer:
    li   s1, 5
    beq  s0, s1, done
    li   s2, 0           # j = 0
inner:
    li   s3, 5
    beq  s2, s3, outer_next
    addi t1, s0, 1        # i+1
    addi t2, s2, 1        # j+1
    mul  t3, t1, t2
    li   t4, 5
    mul  t5, s0, t4        # (i*5 + j) * 4 = word offset
    add  t5, t5, s2
    slli t5, t5, 2
    add  t6, t0, t5
    sw   t3, 0(t6)
    addi s2, s2, 1
    j    inner
outer_next:
    addi s0, s0, 1
    j    outer
done:
    lw   a0, 96(t0)       # table[4][4]
    # result: a0 = 5*5 = 25
    ebreak
