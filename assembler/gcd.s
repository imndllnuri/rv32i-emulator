.globl _start
_start:
    li   t0, 48         # a
    li   t1, 18         # b
loop:
    beqz t1, done
    rem  t2, t0, t1     # RV32M: t2 = a % b
    mv   t0, t1
    mv   t1, t2
    j    loop
done:
    # result: t0 = gcd(48, 18) = 6
    ebreak
