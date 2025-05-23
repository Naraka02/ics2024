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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";
static int expr_len;
static char ops[] = {'+', '-', '*', '/'};

uint32_t choose(uint32_t n) {
	return rand() % n;
}

static void gen_rand_expr() {
  if (expr_len > 10) {
    buf[expr_len++] = choose(10) + '0';
    buf[expr_len++] = 'u';
    return;
  }

  for (int i = 0; i < choose(4); i++) {
    buf[expr_len++] = ' ';
  }

  switch (choose(3)) {
    case 0:
      unsigned randInt = choose(INT8_MAX) + 1;
      sprintf(buf + expr_len, "%u", randInt);
      while (randInt > 0) {
        expr_len++;
        randInt /= 10;
      }
      buf[expr_len++] = 'u';
      break;
    case 1:
      buf[expr_len++] = '(';
      gen_rand_expr();
      buf[expr_len++] = ')';
      break;
    default:
      gen_rand_expr();
      buf[expr_len++] = ops[choose(4)];
      gen_rand_expr();
      break;
  }

  for (int i = 0; i < choose(4); i++) {
    buf[expr_len++] = ' ';
  }
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
		expr_len = 0;
    gen_rand_expr();
    buf[expr_len] = '\0';

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -Wno-div-by-zero -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    if (ret == -1) {
      continue;
    }
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
