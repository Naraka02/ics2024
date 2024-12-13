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

#include "common.h"
#include <isa.h>
#include <memory/paddr.h>
#include <memory/vaddr.h>
#include <stdint.h>

paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type) {
  uint32_t vpn_1 = BITS(vaddr, 31, 22);
  uint32_t vpn_0 = BITS(vaddr, 21, 12);
  uint32_t page_offset = BITS(vaddr, 11, 0);

  uintptr_t updir = cpu.csr.satp << 12;
  uintptr_t updir_pte_addr = updir + vpn_1 * 4;

  word_t updir_pte = paddr_read(updir_pte_addr, 4);
  Log("%x", updir_pte);
  assert(updir_pte & 0x1);

  uintptr_t dir = updir_pte & 0xFFFFF000;
  uintptr_t pte_addr = dir + vpn_0 * 4;

  word_t pte = paddr_read(pte_addr, 4);
  assert(pte & 0x1);

  switch (type) {
  case MEM_TYPE_IFETCH:
    assert(pte & 0x08);
    break;
  case MEM_TYPE_READ:
    assert(pte & 0x02);
    break;
  case MEM_TYPE_WRITE:
    assert(pte & 0x04);
    break;
  }

  uint32_t ppn = pte & 0xFFFFF000;
  paddr_t paddr = ppn | page_offset;
  return paddr;
}
