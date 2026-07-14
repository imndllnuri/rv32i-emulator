.globl _start
_start:
    li   t0, 0x0F0F0F0F  # value to count set bits of (16 ones)
    li   a0, 0           # count
loop:
    beqz t0, done
    andi t1, t0, 1
    add  a0, a0, t1
    srli t0, t0, 1
    j    loop
done:
    # result: a0 = 16 (number of set bits in 0x0F0F0F0F)
    ebreak
