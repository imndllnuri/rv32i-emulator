#ifndef RISCV_FETCH_HPP
#define RISCV_FETCH_HPP

#include <cstdint>

namespace riscv {

class Memory;

namespace fetch {

uint32_t fetch_instruction(uint32_t pc, const Memory &mem);

} // namespace fetch
} // namespace riscv

#endif
