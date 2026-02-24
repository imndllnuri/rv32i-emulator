#ifndef RISCV_FETCH_HPP
#define RISCV_FETCH_HPP

#include <cstdint>

namespace rv32i {

class Memory;

namespace fetch {

uint32_t fetch_instruction(uint32_t pc, const Memory &mem);

} // namespace fetch
} // namespace rv32i

#endif
