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

#ifndef __ISA_RISCV_H__
#define __ISA_RISCV_H__

#include <common.h>

typedef struct {
  word_t mcause, mstatus, mtvec, satp, mscratch;
  vaddr_t mepc;
} MUXDEF(CONFIG_RV64, riscv64_csr, riscv32_csr);

typedef struct {
  word_t gpr[MUXDEF(CONFIG_RVE, 16, 32)];
  vaddr_t pc;
  MUXDEF(CONFIG_RV64, riscv64_csr, riscv32_csr) csr;
  bool INTR;
} MUXDEF(CONFIG_RV64, riscv64_CPU_state, riscv32_CPU_state);

#define MSTATUS_MIE 0x00000008
#define MSTATUS_MPIE 0x00000080
#define MSTATUS_MPP 0x00000a00
// decode
typedef struct {
  union {
    uint32_t val;
  } inst;
} MUXDEF(CONFIG_RV64, riscv64_ISADecodeInfo, riscv32_ISADecodeInfo);

#define isa_mmu_check(vaddr, len, type)                                        \
  (BITS(cpu.csr.satp, 31, 31) ? MMU_TRANSLATE : MMU_DIRECT)

#endif
