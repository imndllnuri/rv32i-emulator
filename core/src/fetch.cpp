#include "../include/fetch.hpp"
#include "../include/memory.hpp"

namespace riscv {
namespace fetch {

uint32_t fetch_instruction(uint32_t pc, const Memory &mem) {
  return mem.read_word(pc);
}

} // namespace fetch
} // namespace riscv
