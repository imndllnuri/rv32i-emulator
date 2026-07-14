.globl _start
_start:
    li   t0, 0x4000      # base address of the array
    li   t1, 3
    sw   t1, 0(t0)
    li   t1, 42
    sw   t1, 4(t0)
    li   t1, 17
    sw   t1, 8(t0)
    li   t1, 8
    sw   t1, 12(t0)
    li   t1, 99
    sw   t1, 16(t0)
    li   t1, 5
    sw   t1, 20(t0)

    lw   a0, 0(t0)       # max = arr[0]
    li   a1, 1           # index
    li   a2, 6           # element count
loop:
    beq  a1, a2, done
    slli t2, a1, 2
    add  t2, t2, t0
    lw   t3, 0(t2)
    ble  t3, a0, skip
    mv   a0, t3
skip:
    addi a1, a1, 1
    j    loop
done:
    # result: a0 = 99 (the max element)
    ebreak
