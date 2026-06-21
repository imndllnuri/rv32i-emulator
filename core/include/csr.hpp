#ifndef RISCV_CSR_HPP
#define RISCV_CSR_HPP

#include <array>
#include <cstdint>

namespace rv32i {

// Flat 12-bit-addressed CSR file. No privilege levels or side-effecting
// CSRs are modeled - reads/writes are plain storage, which is enough for a
// single-hart simulator without an OS underneath it.
class CsrFile {
public:
  CsrFile() = default;

  void reset();
  uint32_t read(uint16_t addr) const;
  void write(uint16_t addr, uint32_t value);

private:
  std::array<uint32_t, 4096> csrs{};
};

} // namespace rv32i

#endif
