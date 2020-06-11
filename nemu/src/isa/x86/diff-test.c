#include "nemu.h"
#include "monitor/diff-test.h"

bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
  if (pc != ref_r->pc) {
    printf("[pc]\tnemu: 0x%x\tqemu: 0x%x\n", pc, ref_r->pc);
    return false;
  }
  for (int i = 0 ; i < 8 ; i++)
    if (reg_l(i) != ref_r->gpr[i]._32) {
      printf("[reg %d]\tnemu: 0x%x\tqemu: 0x%x\n", i, reg_l(i), (uint32_t)ref_r->gpr[i]._32);
      return false;
    }
  return true;
}

void isa_difftest_attach(void) {
}
