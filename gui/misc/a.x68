# sum_1_to_10.s – Simple RV32I program
# Computes sum = 1 + 2 + ... + 10, result in a0

.section .text
.globl _start

_start:
    li a0, 0          # sum = 0
    li t0, 1          # counter i = 1
    li t1, 11         # upper bound (i < 11 → i ≤ 10)

loop:
    add a0, a0, t0    # sum += i
    addi t0, t0, 1    # i++
    blt t0, t1, loop  # if i < 11, continue

halt:
    j halt            # infinite loop to stop execution