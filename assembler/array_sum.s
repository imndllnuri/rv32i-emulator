# No linker is run before objcopy (see RELEASE_PROCESS.md / assembler/README.md),
# so a .data section would land at address 0 and overlap .text. All examples
# in this directory build their "arrays" in memory by hand with sw, at a
# fixed scratch address chosen well clear of the loaded code (0x1000+) and
# the stack (which starts at 0xEFFFF and grows down).
.globl _start
_start:
    li   t0, 0x4000      # base address of the array
    li   t1, 10
    sw   t1, 0(t0)
    li   t1, 20
    sw   t1, 4(t0)
    li   t1, 30
    sw   t1, 8(t0)
    li   t1, 40
    sw   t1, 12(t0)
    li   t1, 50
    sw   t1, 16(t0)

    li   a0, 0           # sum
    li   a1, 5           # element count
    li   a2, 0           # index
loop:
    beq  a2, a1, done
    slli t2, a2, 2
    add  t2, t2, t0
    lw   t3, 0(t2)
    add  a0, a0, t3
    addi a2, a2, 1
    j    loop
done:
    # result: a0 = 10+20+30+40+50 = 150
    ebreak
