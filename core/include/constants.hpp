#ifndef RISCV_CONSTANTS_HPP
#define RISCV_CONSTANTS_HPP

#include <cstdint>

namespace rv32i {

constexpr uint32_t MEMORY_SIZE = 1024 * 1024;
constexpr uint32_t TEXT_START = 0x01000;
constexpr uint32_t STACK_TOP = 0xEFFFF;

} // namespace rv32i

#endif
