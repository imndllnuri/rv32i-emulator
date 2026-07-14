.globl _start
_start:
    li   t0, 0x4000      # base address of the string
    li   t1, 0x48        # 'H'
    sb   t1, 0(t0)
    li   t1, 0x69        # 'i'
    sb   t1, 1(t0)
    li   t1, 0x21        # '!'
    sb   t1, 2(t0)
    sb   zero, 3(t0)     # NUL terminator

    mv   a0, t0
    li   a1, 0           # length
loop:
    lb   t1, 0(a0)
    beqz t1, done
    addi a1, a1, 1
    addi a0, a0, 1
    j    loop
done:
    # result: a1 = 3 (length of "Hi!")
    ebreak
