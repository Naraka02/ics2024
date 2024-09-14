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

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <memory/vaddr.h>

#define TOKEN_SIZE 10005

enum {
	TK_NOTYPE = 256, TK_EQ, TK_NEG,
  TK_INT, TK_HEX, TK_REG, TK_DEREF,
		/* TODO: Add more token types */

};

static struct rule {
	const char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

  {" +", TK_NOTYPE},          // spaces
  {"\\+", '+'},						    // plus
  {"==", TK_EQ},					    // equal
  {"-", '-'},				          // minus or negative
  {"\\*", '*'},						    // multiply or deref
	{"\\/", '/'},						    // divide
  {"0x[0-9a-fA-F]+", TK_HEX}, // hex number
	{"[0-9]+u?", TK_INT},			  // integer
	{"\\(", '('},						    // left bracket
	{"\\)", ')'},							  // right bracket
  {"$[0-9a-zA-Z]+", TK_REG},  //register 
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
	int i;
	char error_msg[128];
	int ret;

	for (i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if (ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct token {
	int type;
	char str[32];
} Token;

static Token tokens[TOKEN_SIZE] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;

	nr_token = 0;

	while (e[position] != '\0') {
		/* Try all rules one by one. */
		for (i = 0; i < NR_REGEX; i ++) {
			if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				//Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
				//		i, rules[i].regex, position, substr_len, substr_len, substr_start);

				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 */

				switch (rules[i].token_type) {
					case '+':
          case '-':
					case '*':
          case '/':
					case '(':
					case ')':
					case TK_EQ:
            tokens[nr_token].type = rules[i].token_type;
						nr_token++;
						break;
					case TK_INT:
          case TK_HEX:
						tokens[nr_token].type = rules[i].token_type;
						if(substr_len > 31) {
							printf("too large integer at position %d\n%s\n%*.s^\n", position, e, position, "");
							return false;
						}
						strcpy(tokens[nr_token].str, substr_start);
						nr_token++;
						break;
          case TK_REG:
            tokens[nr_token].type = rules[i].token_type;
            strcpy(tokens[nr_token].str, substr_start);
            nr_token++;
            break;
					default:
						break;
				}

				break;
			}
		}

		if (i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}


	return true;
}

bool check_parentheses(int start, int end) {
	if(tokens[start].type != '(' || tokens[end].type != ')') {
		return false;
	}	
	/* Use a stack to check parentheses*/
	int top = 0, i = start + 1;
	for(; i < end; i++){
		if(tokens[i].type == '(') {
			top++;
		} else if(tokens[i].type == ')') {
			top--;
		}

		if(top < 0) {
			return false;
		}
	}

	return top == 0 ? true : false;
}

word_t eval(int start, int end, bool *success) {
	if(start > end) {
		/* Bad expression */
		*success = false;
		return 0;
	} else if (start == end) {
		/* Single integer token */
		word_t result;
    if(tokens[start].type == TK_INT) {
		  sscanf(tokens[start].str, "%u", &result);
    } else if(tokens[start].type == TK_HEX) {
      sscanf(tokens[start].str, "%x", &result);
    }
		return result;
	} else if(check_parentheses(start, end) == true) {
		/* throw away the parentheses */
		return eval(start + 1, end - 1, success);
	} else {
		int main_op_pos = -1;
		word_t val1, val2;
    int in_parentheses = 0; // 0 when not in parentheses
		for (int i = start; i <= end; i++) {
      if(tokens[i].type == '(') {
        in_parentheses++;
        continue;
      } else if(tokens[i].type == ')') {
        in_parentheses--;
        continue;
      } else if(in_parentheses > 0) {
        continue;
      }

			switch (tokens[i].type) {
				case '+':
				case '-':
					main_op_pos = i;
					break;
				case '*':
				case '/':
					if(main_op_pos == -1) {
						main_op_pos = i;
					} else if(tokens[main_op_pos].type != '+' && tokens[main_op_pos].type != '-') {
						main_op_pos = i;
					}
					break;
				case TK_NEG:
        case TK_DEREF:
					if(main_op_pos == -1) {
						main_op_pos = i;
					}
					break;
				default:
					break;
				}
		}
		if(main_op_pos == -1) {
			*success = false;
			return 0;
		}

		if(tokens[main_op_pos].type == TK_NEG) {
			return -eval(start + 1, end, success);
		}
    if(tokens[main_op_pos].type == TK_DEREF) {
      return vaddr_read(eval(start + 1, end, success), 4);
    } 

		val1 = eval(start, main_op_pos - 1, success);
		val2 = eval(main_op_pos + 1, end, success);

		switch (tokens[main_op_pos].type){
			case '+': return val1 + val2; break;
			case '-': return val1 - val2; break;
			case '*': return val1 * val2; break;
			case '/': 
        if (val2 == 0) {
          *success = false;
          return 0;
        }
        return val1 / val2;
        break;
			default: *success = false; return 0; break;
		}
	}
}

word_t expr(char *e, bool *success) {
	if (!make_token(e)) {
		*success = false;
		return 0;
	}

  int i;
  for(i = 0; i < nr_token; i++){
    if(tokens[i].type == '-') {
      if(i == 0 || tokens[i - 1].type == '+' || tokens[i - 1].type == '-'
                || tokens[i - 1].type == '*' || tokens[i - 1].type == '/' 
                || tokens[i - 1].type == '(') {
        tokens[i].type = TK_NEG;
      }
    }
    if(tokens[i].type == '*') {
      if(i == 0 || tokens[i - 1].type == '+' || tokens[i - 1].type == '-'
                || tokens[i - 1].type == '*' || tokens[i - 1].type == '/' 
                || tokens[i - 1].type == '(') {
        tokens[i].type = TK_DEREF;
      }
    }
  }

	/* TODO: Insert codes to evaluate the expression. */
	return eval(0, nr_token - 1, success);
}
