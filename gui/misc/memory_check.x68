_start:
    # 1. Store a word (32 bits) at address 0x2000
    li   t0, 0xDEADBEEF          # value to store
    li   t1, 0x2000              # destination address
    sw   t0, 0(t1)               # store word (4 bytes)

    # 2. Store a half-word (16 bits) at address 0x2004
    li   t0, 0x1234               # 16-bit value
    li   t1, 0x2004               # address (must be half-word aligned)
    sh   t0, 0(t1)                # store half-word

    # 3. Store a byte at address 0x2008
    li   t0, 0xAB                 # byte value
    li   t1, 0x2008               # address
    sb   t0, 0(t1)                # store byte

    # 4. Store a sequence of bytes (like a string) at 0x2010
    li   t1, 0x2010
    li   t0, 'H'
    sb   t0, 0(t1)
    li   t0, 'e'
    sb   t0, 1(t1)
    li   t0, 'l'
    sb   t0, 2(t1)
    sb   t0, 3(t1)                # second 'l'
    li   t0, 'o'
    sb   t0, 4(t1)
    li   t0, 0                    # null terminator
    sb   t0, 5(t1)

    # 5. Store a word using unaligned address (optional, may cause exception)
    #    RV32I requires alignment for word accesses; unaligned will trap.
    #    We'll skip it to keep the example clean.

halt:
    ebreak                         # stop execution