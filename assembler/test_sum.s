.globl _start
_start:
    li   t0, 0          # sum = 0
    li   t1, 1          # i = 1
    li   t2, 10         # limit
loop:
    add  t0, t0, t1     # sum += i
    addi t1, t1, 1      # i++
    ble  t1, t2, loop   # if i <= 10, loop
    # result in t0 (should be 55)
    ebreak
