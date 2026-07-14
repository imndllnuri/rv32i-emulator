.globl _start
_start:
    li   t0, 0x4000      # base address of the array
    li   t1, 1
    sw   t1, 0(t0)
    li   t1, 2
    sw   t1, 4(t0)
    li   t1, 3
    sw   t1, 8(t0)
    li   t1, 4
    sw   t1, 12(t0)
    li   t1, 5
    sw   t1, 16(t0)

    mv   a0, t0          # left pointer
    addi a1, t0, 16      # right pointer (last element)
loop:
    bge  a0, a1, done
    lw   t1, 0(a0)
    lw   t2, 0(a1)
    sw   t2, 0(a0)
    sw   t1, 0(a1)
    addi a0, a0, 4
    addi a1, a1, -4
    j    loop
done:
    # result: memory[0x4000..0x4014) = 5, 4, 3, 2, 1
    ebreak
