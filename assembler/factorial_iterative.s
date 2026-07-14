.globl _start
_start:
    li   t0, 1          # result = 1
    li   t1, 1          # i = 1
    li   t2, 7          # compute 7!
loop:
    bgt  t1, t2, done
    mul  t0, t0, t1
    addi t1, t1, 1
    j    loop
done:
    # result: t0 = 7! = 5040
    ebreak
