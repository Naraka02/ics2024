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

#include "sdb.h"
#include <cpu/cpu.h>
#include <isa.h>
#include <memory/vaddr.h>
#include <readline/history.h>
#include <readline/readline.h>

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();
void new_wp(char *e);
void free_wp(int NO);
void print_wp();
void ftrace_display();
void init_ftrace(const char *elf_file);
void difftest_detach();
void difftest_attach();

/* We use the `readline' library to provide more flexibility to read from stdin.
 */
static char *rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;
  return -1;
}

static int cmd_ci(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  uint64_t N;
  int valid, i;

  if (arg == NULL) {
    /* no argument given */
    cpu_exec(1);
  } else {
    valid = sscanf(arg, "%lu", &N);
    for (i = 0; arg[i] != '\0'; i++) {
      if (arg[i] < '0' || arg[i] > '9') {
        valid = 0;
        break;
      }
    }
    if (valid == 1) {
      cpu_exec(N);
    } else {
      /* not a number */
      printf("Invalid argument, please enter a number.\n");
    }
  }

  return 0;
}

static int cmd_info(char *args) {
  /* extract the first argument */
  if (args == NULL) {
    printf("Please enter a valid argument.\n");
    return 0;
  }
  char *arg = strtok(NULL, " ");

  if (strcmp(arg, "r") == 0) {
    isa_reg_display();
  } else if (strcmp(arg, "w") == 0) {
    print_wp();
  } else {
    printf("Please enter a valid argument.\n");
  }
  return 0;
}

static int cmd_x(char *args) {
  char *arg1 = strtok(NULL, " ");
  char *arg2 = strtok(NULL, "\n");

  if (arg1 == NULL || arg2 == NULL) {
    printf("Please enter two vaild arguments.");
  } else {
    word_t N, address, result;
    bool success = true;
    int i;

    if (sscanf(arg1, "%u", &N) != 1) {
      success = false;
    }
    address = expr(arg2, &success);
    if (success == false) {
      printf("Bad expression.\n");
    } else {
      for (i = 0; i < N; i++) {
        if (i % 4 == 0) {
          printf("0x%08x:\t", address + 16 * (i / 4));
        }
        result = vaddr_read(address + 4 * i, 4);
        printf("0x%08x\t", result);
        if (i % 4 == 3 || i == N - 1) {
          printf("\n");
        }
      }
    }
  }

  return 0;
}

static int cmd_p(char *args) {
  bool success = true;
  word_t result = expr(args, &success);
  if (success == false) {
    printf("Bad expression.\n");
  } else {
    printf("%s = %u\n", args, result);
    printf("0x%08x\n", result);
  }
  return 0;
}

static int cmd_w(char *args) {
#ifdef CONFIG_WATCHPOINT
  if (args == NULL) {
    printf("Please enter a valid expression.\n");
    return 0;
  }
  new_wp(args);
#else
  printf("Watchpoint is not enabled.\n");
#endif
  return 0;
}

static int cmd_d(char *args) {
#ifdef CONFIG_WATCHPOINT
  if (args == NULL) {
    printf("Please enter NO of the watchpoint.\n");
    return 0;
  }
  char *arg = strtok(NULL, " ");
  int NO;

  if (sscanf(arg, "%d", &NO) != 1) {
    printf("Please enter NO of the watchpoint.\n");
  } else {
    free_wp(NO);
  }
#else
  printf("Watchpoint is not enabled.\n");
#endif

  return 0;
}

static int cmd_f(char *args) {
#ifdef CONFIG_FTRACE
  printf("Function trace log:\n");
  ftrace_display();
#else
  printf("Function trace is not enabled.\n");
#endif
  return 0;
}

static int cmd_e(char *args) {
#ifdef CONFIG_FTRACE
  if (args == NULL) {
    printf("Please enter a valid ELF file.\n");
    return 0;
  }
  init_ftrace(args);
#else
  printf("Function trace is not enabled.\n");
#endif
  return 0;
}

static int cmd_detach(char *args) {
#ifdef CONFIG_DIFFTEST
  difftest_detach();
#else
  printf("Difftest is not enabled.\n");
#endif
  return 0;
}

static int cmd_attach(char *args) {
#ifdef CONFIG_DIFFTEST
  difftest_attach();
#else
  printf("Difftest is not enabled.\n");
#endif
  return 0;
}

static int cmd_save(char *args) {
  FILE *fp = fopen("snapshot", "w");
  if (fp == NULL) {
    printf("Failed to open snapshot file.\n");
    return 0;
  }
  fwrite(&cpu, sizeof(cpu), 1, fp);
  fclose(fp);
  return 0;
}

static int cmd_load(char *args) {
  FILE *fp = fopen("snapshot", "r");
  if (fp == NULL) {
    printf("Failed to open snapshot file.\n");
    return 0;
  }
  fread(&cpu, sizeof(cpu), 1, fp);
  fclose(fp);
  return 0;
}

static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler)(char *);
} cmd_table[] = {
    {"help", "Display information about all supported commands", cmd_help},
    {"c", "Continue the execution of the program", cmd_c},
    {"q", "Exit NEMU", cmd_q},
    {"si", "Step program N(default by 1) times", cmd_ci},
    {"info", "Print status of registers or watchpoints", cmd_info},
    {"x", "Examine memory", cmd_x},
    {"p", "Calculate the expression", cmd_p},
    {"w", "Set watchpoints", cmd_w},
    {"d", "Delete watchpoints", cmd_d},
    {"e", "Assign an ELF file", cmd_e},
    {"f", "Print the function trace log", cmd_f},
    {"detach", "Detach difftest", cmd_detach},
    {"attach", "Attach difftest", cmd_attach},
    {"save", "Save snapshot", cmd_save},
    {"load", "Load snapshot", cmd_load},
    /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  } else {
    for (i = 0; i < NR_CMD; i++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() { is_batch_mode = true; }

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL;) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) {
      continue;
    }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) {
          return;
        }
        break;
      }
    }

    if (i == NR_CMD) {
      printf("Unknown command '%s'\n", cmd);
    }
  }
}

void test_regex() {
  FILE *fp = fopen("/home/naraka02/ics2024/nemu/tools/gen-expr/input", "r");
  char input_str[1005], input_expr[1005];
  word_t result;
  bool success = true;
  bool test_pass = true;
  while (fgets(input_str, 1005, fp) != NULL) {
    sscanf(input_str, "%u %[^\n]", &result, input_expr);
    word_t expr_res = expr(input_expr, &success);
    if (result != expr_res) {
      test_pass = false;
      printf("Wrong result.\n%s != %u\n", input_expr, expr_res);
      printf("%s = %u\n", input_expr, result);
    }
  }
  fclose(fp);
  if (test_pass) {
    printf("regex test passed\n");
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Test the expressions. */
  // test_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
