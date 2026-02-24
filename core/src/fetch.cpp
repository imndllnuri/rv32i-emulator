#include "../include/fetch.hpp"
#include "../include/memory.hpp"

namespace rv32i {
namespace fetch {

uint32_t fetch_instruction(uint32_t pc, const Memory &mem) {
  return mem.read_word(pc);
}

} // namespace fetch
} // namespace rv32i
