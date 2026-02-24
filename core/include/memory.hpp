#ifndef RISCV_MEMORY_HPP
#define RISCV_MEMORY_HPP

#include "constants.hpp"
#include <array>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace rv32i {

class MemoryAccessException : public std::runtime_error {
public:
  explicit MemoryAccessException(const std::string &msg)
      : std::runtime_error(msg) {}
};

class Memory {
public:
  Memory() : memory{} {}

  void load_program(const std::vector<uint8_t> &code, uint32_t address) {
    if (address + code.size() > MEMORY_SIZE)
      throw MemoryAccessException("Memory is not big enough to load this "
                                  "program try increasing the memory size.");
    std::copy(code.begin(), code.end(), memory.begin() + address);
  }

  uint8_t read_byte(uint32_t addr) const {
    if (addr >= MEMORY_SIZE)
      throw MemoryAccessException("Out of bounds while reading memory byte.");
    return memory[addr];
  }

  uint16_t read_half(uint32_t addr) const {
    if (addr + 1 >= MEMORY_SIZE)
      throw MemoryAccessException("Out of bounds while reading memory half.");
    return static_cast<uint16_t>(memory[addr]) |
           (static_cast<uint16_t>(memory[addr + 1]) << 8);
  }

  uint32_t read_word(uint32_t addr) const {
    if (addr + 3 >= MEMORY_SIZE)
      throw MemoryAccessException("Out of bounds while reading memory word.");
    return static_cast<uint32_t>(memory[addr]) |
           (static_cast<uint32_t>(memory[addr + 1]) << 8) |
           (static_cast<uint32_t>(memory[addr + 2]) << 16) |
           (static_cast<uint32_t>(memory[addr + 3]) << 24);
  }

  void write_byte(uint32_t addr, uint8_t value) {
    if (addr >= MEMORY_SIZE)
      throw MemoryAccessException("Out of bounds while writing memory byte.");
    memory[addr] = value;
  }

  void write_half(uint32_t addr, uint16_t value) {
    if (addr + 1 >= MEMORY_SIZE)
      throw MemoryAccessException("Out of bounds while writing memory half.");
    memory[addr] = value & 0xFF;
    memory[addr + 1] = (value >> 8) & 0xFF;
  }

  void write_word(uint32_t addr, uint32_t value) {
    if (addr + 3 >= MEMORY_SIZE)
      throw MemoryAccessException("Out of bounds while writing memory word.");
    memory[addr] = value & 0xFF;
    memory[addr + 1] = (value >> 8) & 0xFF;
    memory[addr + 2] = (value >> 16) & 0xFF;
    memory[addr + 3] = (value >> 24) & 0xFF;
  }

private:
  std::array<uint8_t, MEMORY_SIZE> memory;
};

} // namespace rv32i

#endif
