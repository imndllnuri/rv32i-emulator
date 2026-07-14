.globl _start
_start:
    li   t0, 0xF0F0F0F0
    li   t1, 0x0F0F0F0F
    and  t2, t0, t1      # t2 = 0x00000000
    or   t3, t0, t1      # t3 = 0xFFFFFFFF
    xor  t4, t0, t1      # t4 = 0xFFFFFFFF

    li   t5, 1
    slli t5, t5, 4        # t5 = 16 (logical shift left)

    li   t6, -8
    srai t6, t6, 1         # t6 = -4 (arithmetic shift right, sign-preserving)

    li   a0, 0x80000000
    srli a0, a0, 28        # a0 = 8 (logical shift right, zero-filling)
    # results: t2=0, t3=0xffffffff, t4=0xffffffff, t5=16, t6=-4, a0=8
    ebreak
