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
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
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
  int valid,i;

  if (arg == NULL) {
    /* no argument given */
		cpu_exec(1);
  } else {
		valid = sscanf(arg, "%lu", &N);
		for(i = 0 ;arg[i] != '\0' ; i++) {
			if(arg[i] < '0' || arg[i] > '9') {
				valid = 0;
				break;
			}
		}
		if(valid == 1) {
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
  char *arg = strtok(NULL, " ");
  
  if (strcmp(arg, "r") == 0) {
    isa_reg_display();
  } else if (strcmp(arg, "w") == 0) {
  } else {
	  printf("Please enter a valid argument.\n");
  }
  return 0;
}

static int cmd_x(char *args) {
	char *arg1 = strtok(NULL, " ");
	char *arg2 = strtok(NULL, " ");
	
	if(arg1 == NULL || arg2 == NULL) {
		printf("Please enter two vaild arguments.");
	} else {
		
	}
	
	return 0;
}

static int cmd_p(char *args) {
	bool success = true;
	word_t result = expr(args, &success);
	if(success == false){
		printf("Bad expression.\n");
	} else{
		printf("%s = %u\n", args, result);
	}
	return 0;
}

static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Step program N(default by 1) times", cmd_ci },
  { "info", "Print status of registers or watchpoints", cmd_info }, 
	{ "x", "Examine memory", cmd_x},
	{ "p", "Calculate the expression", cmd_p},
  /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

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
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void test_regex() {
  FILE *fp = fopen("/home/naraka02/ics2024/nemu/tools/gen-expr/input", "r");
  char input_str[1005], input_expr[1005];
  int result;
  bool success = true;
  bool test_pass = true;
  while (fgets(input_str, 1005, fp) != NULL) {
    sscanf(input_str, "%d %[^\n]", &result, input_expr);
    word_t expr_res = expr(input_expr, &success);
    if (result != expr_res) {
      test_pass = false;
      printf("Wrong result.\n%s != %u\n", input_expr, expr_res);
      printf("%s = %d\n", input_expr, result);
    }
  }
  if (test_pass) {
    printf("regex test passed\n");
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Test the expressions. */
  test_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
