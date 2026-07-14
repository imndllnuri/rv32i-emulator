.globl _start
_start:
    li   a0, 6          # compute 6!
    jal  ra, factorial
    # result: a0 = 6! = 720
    ebreak

# int factorial(int n) -- recurses using the stack (sp), demonstrating a
# real call frame: save ra and n across the recursive call, restore after.
factorial:
    addi sp, sp, -8
    sw   ra, 4(sp)
    sw   a0, 0(sp)
    li   t0, 1
    ble  a0, t0, base_case
    addi a0, a0, -1
    jal  ra, factorial
    lw   t1, 0(sp)       # n saved before the recursive call
    mul  a0, a0, t1
    j    epilogue
base_case:
    li   a0, 1
epilogue:
    lw   ra, 4(sp)
    addi sp, sp, 8
    ret
