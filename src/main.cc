#include <stdint.h>

struct MEMORY
{
  constexpr static uint32_t MAX_MEM_SIZE = 1024 * 64; 
  uint8_t data[MAX_MEM_SIZE];
};

struct CPU68K
{
  uint32_t D[8];
  uint32_t A[8];
  uint32_t PC;
  uint16_t SR;
  int running;
};

int main () {
 
  CPU68K cpu68k;
  MEMORY memory;
  return 0;
}
