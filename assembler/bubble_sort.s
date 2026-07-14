.globl _start
_start:
    li   t0, 0x4000      # base address of the array
    li   t1, 5
    sw   t1, 0(t0)
    li   t1, 3
    sw   t1, 4(t0)
    li   t1, 4
    sw   t1, 8(t0)
    li   t1, 1
    sw   t1, 12(t0)
    li   t1, 2
    sw   t1, 16(t0)

    li   s0, 5           # n
outer:
    li   s1, 0           # i = 0
    li   s2, 0           # swapped = false
    addi s3, s0, -1      # n - 1
inner:
    bge  s1, s3, outer_check
    slli t1, s1, 2
    add  t1, t1, t0
    lw   t2, 0(t1)
    lw   t3, 4(t1)
    ble  t2, t3, no_swap
    sw   t3, 0(t1)
    sw   t2, 4(t1)
    li   s2, 1
no_swap:
    addi s1, s1, 1
    j    inner
outer_check:
    bnez s2, outer
    # result: memory[0x4000..0x4014) = 1, 2, 3, 4, 5 (ascending)
    ebreak
