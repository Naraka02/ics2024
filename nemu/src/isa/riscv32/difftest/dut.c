/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <cpu/difftest.h>
#include "../local-include/reg.h"

bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
  bool success = false;
  success = ref_r->pc == cpu.pc;
  for (int i = 0; i < MUXDEF(CONFIG_RVE, 16, 32); i++) {
    success = success && (ref_r->gpr[i] == gpr(i));
  }
  if (!success) {
    Log("pc last at " FMT_WORD " ", pc);
    if (ref_r->pc != cpu.pc) {
      Log("pc mismatch: 0x%x != 0x%x", ref_r->pc, cpu.pc);
    }
    for (int i = 0; i < MUXDEF(CONFIG_RVE, 16, 32); i++) {
      if (ref_r->gpr[i] != gpr(i)) {
        Log("gpr[%d] mismatch: 0x%x != 0x%x", i, ref_r->gpr[i], gpr(i));
      }
    }
  }
  return success;
}

void isa_difftest_attach() {
}
