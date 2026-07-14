# The core models CSRs as flat, side-effect-free storage (core/include/csr.hpp)
# rather than real privileged control/status registers, so this uses mscratch
# purely as a generic 32-bit scratch cell to demonstrate the CSR instruction
# family: CSRRW (swap), CSRRS/CSRRC (set/clear bits), CSRRWI (write immediate).
.globl _start
_start:
    li     t0, 0x2A
    csrrw  t1, mscratch, t0   # t1 = old mscratch (0), mscratch = 0x2A
    csrrs  t2, mscratch, zero # t2 = 0x2A (read without modifying)

    li     t3, 0x0F
    csrrs  t4, mscratch, t3   # t4 = 0x2A (old value), mscratch |= 0x0F -> 0x2F
    csrrc  t5, mscratch, t3   # t5 = 0x2F (old value), mscratch &= ~0x0F -> 0x20
    csrrwi t6, mscratch, 5    # t6 = 0x20 (old value), mscratch = 5
    # results: t1=0, t2=0x2A, t4=0x2A, t5=0x2F, t6=0x20, final mscratch=5
    ebreak
