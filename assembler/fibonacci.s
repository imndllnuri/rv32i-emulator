.globl _start
_start:
    li   t0, 0          # fib(0)
    li   t1, 1          # fib(1)
    li   t2, 10         # compute fib(10)
    li   t3, 1          # i = 1
loop:
    bge  t3, t2, done
    add  t4, t0, t1     # next = fib(i-1) + fib(i)
    mv   t0, t1
    mv   t1, t4
    addi t3, t3, 1
    j    loop
done:
    # result: t1 = fib(10) = 55
    ebreak
