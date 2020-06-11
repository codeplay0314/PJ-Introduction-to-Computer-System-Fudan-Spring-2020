#include "nemu.h"
#include "monitor/diff-test.h"

bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
  for (int i = 0 ; i < 8 ; i++) {
    if (reg_l(i) != ref_r->gpr[i]._32) {
      printf("[reg %d]\tnemu: 0x%x\tqemu: 0x%x\n", i, reg_l(i), ref_r->gpr[i]._32);
      return false;
    }
  }
  if (reg_flags != ref_r->eflags.val) {
    printf("[Eflags]\tnemu:");
    for (int i = 0; i < 32; i++) printf("%d", reg_flags & (1 << i)? 1: 0);
    printf("B\tqemu:");
    for (int i = 0; i < 32; i++) printf("%d", ref_r->eflags.val & (1 << i)?1: 0);
    printf("B\n");
  }
  return true;
}

void isa_difftest_attach(void) {
}
