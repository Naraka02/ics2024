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

#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  char expr_str[32];
  word_t val;

  /* TODO: Add more members if necessary */

} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
void new_wp(char *e) {
  if (free_ == NULL) {
    Log("No available watchpoints.");
    assert(0);
  }
  bool success = true;
  word_t expr_val = expr(e, &success);
  if (success == false) {
    Log("Bad expression.");
    return;
  }

  WP *new_wp = free_;
  free_ = free_->next;
  new_wp->next = head == NULL ? NULL : head->next;
  head = new_wp;

  strcpy(new_wp->expr_str, e);
  new_wp->val = expr_val;
}

void free_wp(int NO) {
  WP* wp = &wp_pool[NO];
  if (wp == head) {
    head = head->next;
    wp->next = free_;
    free_ = wp;
  } else {
    WP *wp_prev = head;
    while (wp_prev->next != wp) {
      wp_prev = wp_prev->next;
    } 
    wp_prev->next = wp->next;
    wp->next = free_;
    free_ = wp;
  }
}

void print_wp() {
  WP *cur = head;
  printf("%-8s%-12s%-6s%-6s%-12s%s\n", "Num", "Type", "Disp", "Enb", "Address", "What");
  while (cur != NULL) {
    printf("%-8d%-12s%-6s%-6s%-12x%s\n", cur->NO, "breakpoint", "keep", "y", 0, cur->expr_str);
    cur = cur->next;
  }
}

int check_wp() {
  WP *cur = head;
  int hit = 0;
  while (cur != NULL) {
    bool success = true;
    word_t new_val = expr(cur->expr_str, &success);
    if (new_val != cur->val) {
      hit = 1;
      printf("\nWatchpoint %d: %s\n\n", cur->NO, cur->expr_str);
      printf("Old value = %u\n", cur->val);
      printf("New value = %u\n", new_val);
      cur->val = new_val;
    }
    cur = cur->next;
  }
  
  return hit;
}
