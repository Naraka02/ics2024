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

enum {
	TK_NOTYPE = 256, TK_EQ, TK_MINUS,
	TK_NEG, TK_INT
		/* TODO: Add more token types */

};

static struct rule {
	const char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{" +", TK_NOTYPE},			// spaces
	{"\\+", '+'},						// plus
	{"==", TK_EQ},					// equal
	{"-", TK_MINUS},				// minus
	{"-", TK_NEG},					// negative
	{"\\*", '*'},						// multiply
	{"\\/", '/'},						// divide
	{"[0-9]+", TK_INT},			// integer
	{"\\(", '('},						// left bracket
	{"\\)", ')'},							// right bracket
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

static Token tokens[10005] __attribute__((used)) = {};
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
						tokens[nr_token].type = '+';
						nr_token++;
						break;
					case '*':
						tokens[nr_token].type = '*';
						nr_token++;
						break;
          case '/':
            tokens[nr_token].type = '/';
            nr_token++;
            break;
					case '(':
						tokens[nr_token].type = '(';
						nr_token++;
						break;
					case ')':
						tokens[nr_token].type = ')';
						nr_token++;
						break;
					case TK_EQ:
						tokens[nr_token].type = TK_EQ;
						nr_token++;
						break;
					case TK_MINUS:
					case TK_NEG:
						if(nr_token == 0 || 
              ( tokens[nr_token - 1].type != TK_INT && tokens[nr_token - 1] .type != ')')) {
							tokens[nr_token].type = TK_NEG;
						} else {
							tokens[nr_token].type = TK_MINUS;
						}
						nr_token++;
						break;
					case TK_INT:
						tokens[nr_token].type = TK_INT;
						if(substr_len > 31) {
							printf("too large integer at position %d\n%s\n%*.s^\n", position, e, position, "");
							return false;
						}
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
		sscanf(tokens[start].str, "%u", &result);
		return result;
	} else if(check_parentheses(start, end) == true) {
		/* throw away the parentheses */
		return eval(start + 1, end - 1, success);
	} else {
		int main_op_pos = -1;
		word_t val1, val2;
		for (int i = start; i <= end; i++){
			switch (tokens[i].type) {
				case '+':
				case TK_MINUS:
					main_op_pos = i;
					break;
				case '*':
				case '/':
					if( main_op_pos == -1) {
						main_op_pos = i;
					} else if( tokens[i].type != '+' && tokens[i].type != TK_MINUS) {
						main_op_pos = i;
					}
					break;
				case TK_NEG:
					if( main_op_pos == -1) {
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
    printf("%d\n", main_op_pos);
		if(tokens[main_op_pos].type == TK_NEG){
			return -eval(start + 1, end, success);
		}
		val1 = eval(start, main_op_pos - 1, success);
		val2 = eval(main_op_pos + 1, end, success);
    printf("%u %u\n",val1,val2);

		switch (tokens[main_op_pos].type){
			case '+': return val1 + val2; break;
			case TK_MINUS: return val1 - val2; break;
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

	/* TODO: Insert codes to evaluate the expression. */
	return eval(0, nr_token - 1, success);
}
