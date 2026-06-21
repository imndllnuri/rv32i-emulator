.section .data
source:
    .asciz "NURI"

.section .text
.globl _start

_start:
    # -------------------------
    # Add 1 to 9 and store result
    # -------------------------
    li   t0, 9          # t0 = 9
    addi t0, t0, 1      # t0 = t0 + 1 = 10

    li   t1, 0x2000
    sw   t0, 0(t1)      # MEM[0x2000] = 10

    # -------------------------
    # Copy string "NURI"
    # source -> 0x2010
    # -------------------------
    la   t0, source     # source pointer
    li   t1, 0x2010     # destination pointer

copy_loop:
    lb   t2, 0(t0)      # load byte
    sb   t2, 0(t1)      # store byte

    beqz t2, done_copy  # stop when '\0'

    addi t0, t0, 1      # next source char
    addi t1, t1, 1      # next destination char
    j    copy_loop

done_copy:
    ebreak