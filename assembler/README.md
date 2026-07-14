# Example programs

Each `.s` file here is a small, self-contained RISC-V program you can open in
the GUI (`python gui/main.py`, or `File > Open`) and run with **Assemble**
then **Run** (or **Step** through it instruction by instruction). There's no
OS underneath the emulator, so none of these print output — instead, each
program ends in `ebreak` and leaves its result sitting in a register (visible
in the Registers dock) or in memory at a fixed scratch address (visible in
the Memory dock, jump to the address noted in the comment).

Every example uses only RV32I/RV32M instructions and a flat `.text` section
— **no `.data`/`.bss`**. The GUI assembles with `as` and then runs `objcopy
-O binary` directly on the unlinked object file (no `ld` step), and an
unlinked object's sections all start at address 0; a separate `.data`
section would land at address 0 too and silently overwrite the start of
`.text` in the flattened binary. Anywhere an example needs an "array" or
"string", it builds it by hand with `sw`/`sb` at a fixed address (`0x4000`),
chosen well clear of where code loads (`0x1000` onward) and the stack
(starts at `0xEFFFF`, grows down).

| File | Demonstrates | Result |
|---|---|---|
| `test_sum.s` | loop, `add`, `ble` branch | `t0` = sum of 1..10 = 55 |
| `fibonacci.s` | iterative loop, register rotation | `t1` = fib(10) = 55 |
| `factorial_iterative.s` | loop, `mul` (RV32M) | `t0` = 7! = 5040 |
| `factorial_recursive.s` | `jal`/`ret`, stack frame (`sp`, saved `ra`) | `a0` = 6! = 720 |
| `gcd.s` | `rem` (RV32M), Euclidean algorithm | `t0` = gcd(48, 18) = 6 |
| `array_sum.s` | building an array with `sw`, indexed `lw` | `a0` = 150 |
| `array_max.s` | linear scan, `ble` compare-and-select | `a0` = 99 |
| `array_reverse.s` | two-pointer in-place swap | `mem[0x4000..0x4014)` = 5,4,3,2,1 |
| `bubble_sort.s` | nested loops, swap-flag termination | `mem[0x4000..0x4014)` = 1,2,3,4,5 |
| `string_length.s` | byte loads/stores (`lb`/`sb`), NUL scan | `a1` = 3 (length of "Hi!") |
| `multiplication_table.s` | nested loops, 2D index arithmetic, `mul` | `a0` = table[4][4] = 25 |
| `bitwise_ops.s` | `and`/`or`/`xor`, logical vs. arithmetic shifts | see comments in the file |
| `count_set_bits.s` | bit-at-a-time popcount | `a0` = 16 |
| `csr_scratch.s` | `csrrw`/`csrrs`/`csrrc`/`csrrwi` | see comments in the file |

`csr_scratch.s` uses `mscratch` purely as a generic 32-bit scratch cell —
this emulator models CSRs as flat, side-effect-free storage
(`core/include/csr.hpp`), not real privileged control/status registers.
