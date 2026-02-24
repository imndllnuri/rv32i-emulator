#include "./include/cpu.hpp"
#include <fstream>
#include <iostream>
#include <vector>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <binary file>\n";
    return 1;
  }

  // Open the binary file
  std::ifstream file(argv[1], std::ios::binary | std::ios::ate);
  if (!file) {
    std::cerr << "Error: cannot open file " << argv[1] << '\n';
    return 1;
  }

  // Get file size and read contents
  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);
  std::vector<uint8_t> buffer(size);
  if (!file.read(reinterpret_cast<char *>(buffer.data()), size)) {
    std::cerr << "Error: failed to read file\n";
    return 1;
  }

  // Create and initialize CPU
  rv32i::CPU cpu;
  cpu.reset();

  // Load program at TEXT_START (0x1000)
  cpu.load_program(buffer, cpu.TEXT_START);

  std::cout << "Running program from 0x" << std::hex << cpu.TEXT_START
            << std::dec << " (" << size << " bytes)\n";

  // Execute until EBREAK or error
  cpu.run();

  // Print final register state
  auto regs = cpu.registers_state();
  std::cout << "\nExecution finished. Register dump:\n";
  for (int i = 0; i < 32; ++i) {
    std::cout << "x" << i << " = 0x" << std::hex << regs[i] << std::dec;
    if (i % 4 == 3)
      std::cout << '\n';
    else
      std::cout << '\t';
  }
  std::cout << "PC = 0x" << std::hex << cpu.get_pc() << std::dec << '\n';

  return 0;
}
