#include "nemu.h"
#include <stdlib.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>  // for c languare, regularized expressions

enum  TK{
  TK_NOTYPE = 256, TK_EQ = 257,
  /* TODO: Add more token types */
  TK_DEC = 10, TK_HEX = 16
};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {"\\+", '+'},               // plus
  {"\\-", '-'},               // minus
  {"\\*", '*'},               // mutiply
  {"\\/", '/'},               // devide
  {"\\(", '('},               // left parenthesis
  {"\\)", ')'},               // right parenthesis
  {" +", TK_NOTYPE},          // spaces
  {"==", TK_EQ},              // equal
  {"[0-9]+", TK_DEC},         // decimal number
  {"0x[0-9a-f]+", TK_HEX}     // hexadecimal number
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX] = {}; // regex_t store number of regexs

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    // regcomp(regex_t *preg, const char * regex, int cflags)，description of function regcomp
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

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0; // number of regex tokens

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_DEC:
          case TK_HEX:
          case '+':
          case '-':
          case '*':
          case '/':
          case '(':
          case ')':
          case TK_EQ: {
            tokens[nr_token].type = rules[i].token_type;
            strncpy(tokens[nr_token++].str, substr_start, substr_len);
          }break;
          case TK_NOTYPE: break;
          default: return false;
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
  if (tokens[start].type != '(' || tokens[end].type != ')') return false;

  int i, cnt;
  for (i = start + 1, cnt = 1; i <= end; i++) {
    enum TK tk = tokens[i].type;
    if (tk == '(') cnt++;
    else if (tk == ')') {
      --cnt;
      if (cnt == -1) return false;
      assert(cnt > -1);
    }
  }

  return !cnt;
}

int eval(int start, int end, bool *success) {
  printf("[%d %d]\n", start, end);
  if (start > end) {
    *success = false;
    return 0;
  }
  else if (start == end) {
    /* Single token.
     * For now this token should be a number.
     * Return the value of the number.
     */
   int res = 0;
   if (tokens[start].type == TK_DEC) sscanf(tokens[start].str, "%d", &res);
   else if (tokens[start].type == TK_HEX) sscanf(tokens[start].str, "%x", &res);
   else *success = false;
   return res;
  }
  else if (check_parentheses(start, end) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     * '('<expr>')' = <expr>
     */
    return eval(start + 1, end - 1, success);
  }
  else {
    /* We should do more things here. */
    int mainop = start;
    int i, cnt;
    for (i = start, cnt = 0; i <= end; i++) {
      enum TK tk = tokens[i].type;
      if (tk == '(') cnt++;
      else if (tk == ')') cnt--;
      else if (!cnt) {
        printf("- %d -\n", i);
        if (tk == TK_EQ) {
          mainop = i;
          break;
        }
        else if (tk == '+' || tk == '-') mainop = i;
        else {
          if (tokens[mainop].type != '+' && tokens[mainop].type != '-') mainop = i;
        }
      }
    }

    printf("---%d---\n", tokens[mainop].type);
    switch (tokens[mainop].type) {
      case '+': return eval(start, mainop - 1, success) + eval(mainop + 1, end, success);
      case '-': return eval(start, mainop - 1, success) - eval(mainop + 1, end, success);
      case '*': return eval(start, mainop - 1, success) * eval(mainop + 1, end, success);
      case '/': return eval(start, mainop - 1, success) / eval(mainop + 1, end, success);
      case TK_EQ: return eval(start, mainop - 1, success) == eval(mainop + 1, end, success);
      default: *success = 0;
    }

    return 0;
  }
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  int res = eval(0, nr_token - 1, success);
  if (*success == false)
    printf("Please enter valid expression\n");
  return res;
}
