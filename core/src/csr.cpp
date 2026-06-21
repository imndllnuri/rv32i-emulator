#include "../include/csr.hpp"

namespace rv32i {

void CsrFile::reset() {
  for (auto &c : csrs)
    c = 0;
}

uint32_t CsrFile::read(uint16_t addr) const { return csrs[addr & 0xFFF]; }

void CsrFile::write(uint16_t addr, uint32_t value) {
  csrs[addr & 0xFFF] = value;
}

} // namespace rv32i
