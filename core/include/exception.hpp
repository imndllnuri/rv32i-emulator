#include <stdexcept>
#include <string>

namespace riscv {

class CpuException : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

class IllegalInstructionException : public CpuException {
public:
  explicit IllegalInstructionException(const std::string &msg)
      : CpuException("Illegal instruction: " + msg) {}
};

class UnimplementedInstructionException : public CpuException {
public:
  explicit UnimplementedInstructionException(const std::string &msg)
      : CpuException("Unimplemented instruction: " + msg) {}
};

// Add more as needed: MisalignedAccess, PrivilegedInstruction, etc.

} // namespace riscv
