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

#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <cpu/difftest.h>
#include <locale.h>

/* The assembly code of instructions executed is only output to the screen
 * when the number of instructions executed is less than this value.
 * This is useful when you use the `si' command.
 * You can modify this value as you want.
 */
#define MAX_INST_TO_PRINT 10
#define IRINGBUF_SIZE 16
#define INST_NAME_OFFSET 24

typedef struct {
  char *name;
  word_t from_addr, to_addr;
  bool is_call;
} func_log;

CPU_state cpu = {};
uint64_t g_nr_guest_inst = 0;
static uint64_t g_timer = 0; // unit: us
static bool g_print_step = false;
static char iringbuf[IRINGBUF_SIZE][128];
int iringbuf_idx = 0;
func_log ftrace_log[65536];
int ftrace_log_idx = 0;

void device_update();
int check_wp();
void sdb_mainloop();
char* get_func_name(word_t addr);

static void trace_and_difftest(Decode *_this, vaddr_t dnpc) {
#ifdef CONFIG_ITRACE_COND
  if (ITRACE_COND) { log_write("%s\n", _this->logbuf); }
#endif
  if (g_print_step) { IFDEF(CONFIG_ITRACE, puts(_this->logbuf)); }
  IFDEF(CONFIG_DIFFTEST, difftest_step(_this->pc, dnpc));
#ifdef CONFIG_WATCHPOINT
  if (check_wp() == 1 && nemu_state.state != NEMU_END) {
    nemu_state.state = NEMU_STOP;
  }
#endif

#ifdef CONFIG_IRINGBUF
  strcpy(iringbuf[iringbuf_idx], _this->logbuf);
  if (iringbuf_idx < IRINGBUF_SIZE - 1) {
    iringbuf_idx++;
  } else {
    iringbuf_idx = 0;
  }
#endif

#ifdef CONFIG_FTRACE
  if (strncmp((_this->logbuf + INST_NAME_OFFSET), "jal", 3) == 0) {
    ftrace_log[ftrace_log_idx].from_addr = _this->pc;
    ftrace_log[ftrace_log_idx].to_addr = dnpc;
    ftrace_log[ftrace_log_idx].is_call = true;
    ftrace_log[ftrace_log_idx].name = get_func_name(dnpc);
    ftrace_log_idx++;
  } else if (strncmp((_this->logbuf + INST_NAME_OFFSET), "ret", 3) == 0) {
    ftrace_log[ftrace_log_idx].from_addr = _this->pc;
    ftrace_log[ftrace_log_idx].to_addr = dnpc;
    ftrace_log[ftrace_log_idx].is_call = false;
    ftrace_log[ftrace_log_idx].name = get_func_name(_this->pc);
    ftrace_log_idx++;
  }
#endif
}

static void exec_once(Decode *s, vaddr_t pc) {
  s->pc = pc;
  s->snpc = pc;
  isa_exec_once(s);
  cpu.pc = s->dnpc;
#ifdef CONFIG_ITRACE
  char *p = s->logbuf;
  p += snprintf(p, sizeof(s->logbuf), FMT_WORD ":", s->pc);
  int ilen = s->snpc - s->pc;
  int i;
  uint8_t *inst = (uint8_t *)&s->isa.inst.val;
  for (i = ilen - 1; i >= 0; i --) {
    p += snprintf(p, 4, " %02x", inst[i]);
  }
  int ilen_max = MUXDEF(CONFIG_ISA_x86, 8, 4);
  int space_len = ilen_max - ilen;
  if (space_len < 0) space_len = 0;
  space_len = space_len * 3 + 1;
  memset(p, ' ', space_len);
  p += space_len;

  void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
  disassemble(p, s->logbuf + sizeof(s->logbuf) - p,
      MUXDEF(CONFIG_ISA_x86, s->snpc, s->pc), (uint8_t *)&s->isa.inst.val, ilen);
#endif
}

static void execute(uint64_t n) {
  Decode s;
  for (;n > 0; n --) {
    exec_once(&s, cpu.pc);
    g_nr_guest_inst ++;
    trace_and_difftest(&s, cpu.pc);
    if (nemu_state.state != NEMU_RUNNING) break;
    IFDEF(CONFIG_DEVICE, device_update());
  }
}

#ifdef CONFIG_FTRACE
void ftrace_display() {
  int depth = 0;
  for (int i = 0; i < ftrace_log_idx; i++) {
    printf("0x%08x: ", ftrace_log[i].from_addr);
    if (ftrace_log[i].is_call) {
      for (int j = 0; j < depth; j++) {
        printf("  ");
      }
      printf("call [%s@0x%08x]\n", ftrace_log[i].name, ftrace_log[i].to_addr);
      depth++;
    } else {
      depth--;
      for (int j = 0; j < depth; j++) {
        printf("  ");
      }
      printf("ret [%s]\n", ftrace_log[i].name);
    }
  }
}
#endif

static void statistic() {
  IFNDEF(CONFIG_TARGET_AM, setlocale(LC_NUMERIC, ""));
#define NUMBERIC_FMT MUXDEF(CONFIG_TARGET_AM, "%", "%'") PRIu64
  Log("host time spent = " NUMBERIC_FMT " us", g_timer);
  Log("total guest instructions = " NUMBERIC_FMT, g_nr_guest_inst);
  if (g_timer > 0) Log("simulation frequency = " NUMBERIC_FMT " inst/s", g_nr_guest_inst * 1000000 / g_timer);
  else Log("Finish running in less than 1 us and can not calculate the simulation frequency");
}

void iringbuf_display() {
  for (int i = 0; i < IRINGBUF_SIZE; i++) {
    if (strlen(iringbuf[i]) == 0) break;
    if (i == (iringbuf_idx - 1 + IRINGBUF_SIZE) % IRINGBUF_SIZE) {
      printf("--> %s\n", iringbuf[i]);
    } else {
      printf("    %s\n", iringbuf[i]);
    }
  }
}

void assert_fail_msg() {
  isa_reg_display();
  statistic();
  IFDEF(CONFIG_IRINGBUF, iringbuf_display());
}

/* Simulate how the CPU works. */
void cpu_exec(uint64_t n) {
  g_print_step = (n < MAX_INST_TO_PRINT);
  switch (nemu_state.state) {
    case NEMU_END: case NEMU_ABORT: case NEMU_QUIT:
      printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
      return;
    default: nemu_state.state = NEMU_RUNNING;
  }

  uint64_t timer_start = get_time();

  execute(n);

  uint64_t timer_end = get_time();
  g_timer += timer_end - timer_start;

  switch (nemu_state.state) {
    case NEMU_RUNNING: nemu_state.state = NEMU_STOP; break;

    case NEMU_END: case NEMU_ABORT:
      Log("nemu: %s at pc = " FMT_WORD,
          (nemu_state.state == NEMU_ABORT ? ANSI_FMT("ABORT", ANSI_FG_RED) :
           (nemu_state.halt_ret == 0 ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) :
            ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))),
          nemu_state.halt_pc);
      // fall through
    case NEMU_QUIT: statistic();
  }
}
