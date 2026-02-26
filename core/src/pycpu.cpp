#include "../include/cpu.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

class PyCPU {
public:
  PyCPU() : cpu() {}

  void load_program(const std::vector<uint8_t> &code,
                    uint32_t address = rv32i::TEXT_START) {
    cpu.load_program(code, address);
  }

  bool step() { return cpu.step(); }
  void run() { cpu.run(); }
  void reset() { cpu.reset(); }

  std::array<uint32_t, 32> get_registers() const {
    return cpu.registers_state();
  }

  uint32_t get_pc() const { return cpu.get_pc(); }

  uint8_t read_memory_byte(uint32_t addr) const {
    return cpu.read_memory_byte(addr);
  }
  uint16_t read_memory_half(uint32_t addr) const {
    return cpu.read_memory_half(addr);
  }
  uint32_t read_memory_word(uint32_t addr) const {
    return cpu.read_memory_word(addr);
  }

  void write_memory_byte(uint32_t addr, uint8_t value) {
    cpu.write_memory_byte(addr, value);
  }
  void write_memory_half(uint32_t addr, uint16_t value) {
    cpu.write_memory_half(addr, value);
  }
  void write_memory_word(uint32_t addr, uint32_t value) {
    cpu.write_memory_word(addr, value);
  }

private:
  rv32i::CPU cpu;
};

PYBIND11_MODULE(rv32i_core, m) {
  m.doc() = "RV32I CPU core bindings";

  py::class_<PyCPU>(m, "CPU")
      .def(py::init<>())
      .def("load_program", &PyCPU::load_program, py::arg("code"),
           py::arg("address") = rv32i::TEXT_START)
      .def("step", &PyCPU::step)
      .def("run", &PyCPU::run)
      .def("reset", &PyCPU::reset)
      .def("get_registers", &PyCPU::get_registers)
      .def("get_pc", &PyCPU::get_pc)
      .def("read_memory_byte", &PyCPU::read_memory_byte)
      .def("read_memory_half", &PyCPU::read_memory_half)
      .def("read_memory_word", &PyCPU::read_memory_word)
      .def("write_memory_byte", &PyCPU::write_memory_byte)
      .def("write_memory_half", &PyCPU::write_memory_half)
      .def("write_memory_word", &PyCPU::write_memory_word);
}
