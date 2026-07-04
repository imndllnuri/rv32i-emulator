// Regression test for a real bug: Memory::read_half/read_word/write_half/
// write_word used to bounds-check with `addr + N >= MEMORY_SIZE`, which
// overflows uint32_t and wraps past 0 for addr near UINT32_MAX, silently
// bypassing the check and indexing the backing std::array out of range
// (observed as a segfault when reading a near-UINT32_MAX address from the
// GUI/Python side). The fix rewrites the checks as `addr > MEMORY_SIZE - N`,
// which cannot overflow since MEMORY_SIZE is a small compile-time constant.
#include "../include/cpu.hpp"
#include "../include/memory.hpp"
#include <iostream>

#define TEST(cond, msg)                                                        \
  do {                                                                         \
    if (!(cond)) {                                                             \
      std::cerr << "FAIL: " << msg << " (" << __FILE__ << ":" << __LINE__      \
                << ")" << std::endl;                                           \
      return 1;                                                                \
    }                                                                          \
  } while (0)

int main() {
  using namespace rv32i;

  CPU cpu;
  cpu.reset();

  bool threw = false;
  try {
    cpu.read_memory_word(0xFFFFFFFF);
  } catch (const MemoryAccessException &) {
    threw = true;
  }
  TEST(threw, "read_memory_word(0xFFFFFFFF) must throw, not overflow/segfault");

  threw = false;
  try {
    cpu.read_memory_half(0xFFFFFFFF);
  } catch (const MemoryAccessException &) {
    threw = true;
  }
  TEST(threw, "read_memory_half(0xFFFFFFFF) must throw, not overflow/segfault");

  threw = false;
  try {
    cpu.write_memory_word(0xFFFFFFFE, 0x12345678);
  } catch (const MemoryAccessException &) {
    threw = true;
  }
  TEST(threw, "write_memory_word(0xFFFFFFFE, ...) must throw, not overflow");

  threw = false;
  try {
    cpu.write_memory_half(0xFFFFFFFF, 0xABCD);
  } catch (const MemoryAccessException &) {
    threw = true;
  }
  TEST(threw, "write_memory_half(0xFFFFFFFF, ...) must throw, not overflow");

  // Sanity: the last valid word/half in memory must still be reachable.
  cpu.write_memory_word(CPU::MEMORY_SIZE - 4, 0xCAFEBABE);
  TEST(cpu.read_memory_word(CPU::MEMORY_SIZE - 4) == 0xCAFEBABE,
       "last valid word in memory is still readable/writable");

  threw = false;
  try {
    cpu.read_memory_word(CPU::MEMORY_SIZE - 3); // one byte past the last valid word
  } catch (const MemoryAccessException &) {
    threw = true;
  }
  TEST(threw, "reading a word that runs one byte past the end must throw");

  std::cout << "All memory-bounds tests PASSED!" << std::endl;
  return 0;
}
