/***************************************************************************************
 * Copyright (c) 2014-2022 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan
 *PSL v2. You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 *KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 *NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#include "local-include/reg.h"
#include <isa.h>

const char *regs[] = {"$0", "ra", "sp",  "gp",  "tp", "t0", "t1", "t2",
                      "s0", "s1", "a0",  "a1",  "a2", "a3", "a4", "a5",
                      "a6", "a7", "s2",  "s3",  "s4", "s5", "s6", "s7",
                      "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};
const int NR_REGS = sizeof(regs) / sizeof(char *);

void isa_reg_display() {
  int i;
  for (i = 0; i < NR_REGS; i++) {
    if (i == 1 || i == 2 || i == 3 || i == 8 || i == 9) {
      printf("%-10s\t0x%-10x\t0x%x\n", regs[i], cpu.gpr[i], cpu.gpr[i]);
    } else {
      printf("%-10s\t0x%-10x\t%u\n", regs[i], cpu.gpr[i], cpu.gpr[i]);
    }
  }
  printf("%-10s\t0x%-10x\t0x%x\n", "pc", cpu.pc, cpu.pc);
  printf("%-10s\t0x%-10x\t0x%x\n", "mstatus", cpu.csr.mstatus, cpu.csr.mstatus);
  printf("%-10s\t0x%-10x\t0x%x\n", "mscratch", cpu.csr.mscratch,
         cpu.csr.mscratch);
  printf("%-10s\t0x%-10x\t0x%x\n", "mepc", cpu.csr.mepc, cpu.csr.mepc);
  printf("%-10s\t0x%-10x\t0x%x\n", "mcause", cpu.csr.mcause, cpu.csr.mcause);
  printf("%-10s\t0x%-10x\t0x%x\n", "mtvec", cpu.csr.mtvec, cpu.csr.mtvec);
}

word_t isa_reg_str2val(const char *s, bool *success) {
  int i;

  if (strcmp(s + 1, "pc") == 0) {
    return cpu.pc;
  }
  for (i = 0; i < NR_REGS; i++) {
    if (strcmp(s + 1, regs[i]) == 0) {
      return cpu.gpr[i];
    }
  }

  if (i == NR_REGS) {
    *success = false;
  }
  return 0;
}
