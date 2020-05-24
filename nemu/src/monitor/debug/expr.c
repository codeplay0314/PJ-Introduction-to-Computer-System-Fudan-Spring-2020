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
  TK_UEQ = 258, TK_MINUS = 259,
  TK_DEC = 10, TK_HEX = 16,
  TK_REG = 260, TK_POINTER = 261
};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},                        // spaces
  {"\\+", '+'},                             // plus
  {"\\-", '-'},                             // minus
  {"\\*", '*'},                             // mutiply
  {"\\/", '/'},                             // devide
  {"==", TK_EQ},                            // equal
  {"!=", TK_UEQ},                           // unequal
  {"&&", '&'},                              // and
  {"\\|\\|", '|'},                          // or
  {"!", '!'},                               // not
  {"0x[0-9a-f]+", TK_HEX},                  // hexadecimal number
  {"[0-9]+", TK_DEC},                       // decimal number
  {"\\$[a-z]{2,3}", TK_REG},            // register
  {"\\(", '('},                             // left parenthesis
  {"\\)", ')'}                              // right parenthesis
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

#define token_capacity 200
static Token tokens[token_capacity] __attribute__((used)) = {};
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
        // puts("--");
        // for (int cc = 0; cc < substr_len; cc++)
        //   putchar(substr_start[cc]);
        // putchar('\n');
        switch (rules[i].token_type) {
          case '+':
          case '/':
          case TK_EQ:
          case TK_UEQ:
          case '&':
          case '|':
          case '!':
          case TK_DEC:
          case TK_HEX:
          case TK_REG:
          case '(':
          case ')': {
            if (nr_token >= token_capacity)
              panic("Too many tokens for the expression!");
            tokens[nr_token].type = rules[i].token_type;
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token++].str[substr_len] = 0;
          }break;
          case '-':
          case '*': {
            if (nr_token >= token_capacity)
              panic("Too many tokens for the expression!");
            if (!nr_token || (tokens[nr_token - 1].type != TK_DEC && tokens[nr_token - 1].type != TK_HEX \
              && tokens[nr_token - 1].type != TK_REG && tokens[nr_token - 1].type != ')'))
                tokens[nr_token].type = rules[i].token_type == '*'? TK_POINTER: TK_MINUS;
            else
              tokens[nr_token].type = rules[i].token_type;
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token++].str[substr_len] = 0;
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
  // for (int i = 0; i < nr_token; i++)
  //   printf("%s", tokens[i].str);
  // putchar('\n');
  return true;
}

bool check_parentheses(int start, int end, bool *success) {
  if (tokens[start].type != '(' || tokens[end].type != ')') return false;

  int i, cnt;
  for (i = start + 1, cnt = 1; i < end; i++) {
    enum TK tk = tokens[i].type;
    if (tk == '(') cnt++;
    else if (tk == ')') {
      --cnt;
      if (cnt < 0) {
        *success = false;
        return false;
      }
      if (!cnt)
        return false;
    }
  }

  return cnt == 1;
}

int eval(int start, int end, bool *success) {
  printf("[%d %d]\n", start, end);
  if (start > end) {
    *success = false;
    return 0;
  }
  else if (start == end) {
    /* Single token.
     * For now this token should be a number or register.
     * Return the value of the number.
     */

   //printf("--%s--\n", tokens[start].str);
   const char *regsl[] = {"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"};
   const char *regsw[] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
   const char *regsb[] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};
   int res = 0;
   if (tokens[start].type == TK_DEC) sscanf(tokens[start].str, "%d", &res);
   else if (tokens[start].type == TK_HEX) sscanf(tokens[start].str, "%x", &res);
   else if (tokens[start].type == TK_REG) {
     bool ok = false;
     if (!strcmp(tokens[start].str + 1, "pc")) res = isa_vaddr_read(cpu.pc, 8), ok = true;
     else {
       for (int i = 0; i < 8; i++) {
         if (!strcmp(tokens[start].str + 1, regsl[i])) {
           ok = true;
           res = reg_l(i);
           break;
         }
         if (!strcmp(tokens[start].str + 1, regsw[i])) {
           ok = true;
           res = reg_w(i);
           break;
         }
         if (!strcmp(tokens[start].str + 1, regsb[i])) {
           ok = true;
           res = reg_b(i);
           break;
         }
       }
     }
     if (!ok) {
       printf("\"%s\" is not a valid register name. ", tokens[start].str + 1);
       *success = false;
     }
   }
   else *success = false;
   return res;
  }
  else if (check_parentheses(start, end, success) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     * '('<expr>')' = <expr>
     */

    return eval(start + 1, end - 1, success);
  }
  else if (*success) {
    /* We should do more things here. */
    int mainop = start;
    int i, cnt;
    for (i = start, cnt = 0; i <= end; i++) {
      enum TK tk = tokens[i].type;
      if (tk == '(') cnt++;
      else if (tk == ')') cnt--;
      else if (!cnt) {
        if (tk == TK_EQ || tk == TK_UEQ) {
          mainop = i;
          break;
        }
        if (tk == TK_POINTER) continue;
        else if (tk == '&' || tk == '|') {
          if (tokens[mainop].type != '&' && tokens[mainop].type != '|') mainop = i;
        }
        else if (tk == '+' || tk == '-') {
          if (tokens[mainop].type != '&' && tokens[mainop].type != '|' && \
            tokens[mainop].type != '+' && tokens[mainop].type != '-') mainop = i;
        }
        else if (tk == '*' || tk == '/') {
          if (tokens[mainop].type != '&' && tokens[mainop].type != '|' && \
            tokens[mainop].type != '+' && tokens[mainop].type != '-' && \
              tokens[mainop].type != '*' && tokens[mainop].type != '/') mainop = i;
        }
      }
    }

    printf("---%d---\n", mainop);
    int res = 0;
    if (mainop == start) {
      res = eval(start + 1, end, success);
      if (*success) {
        switch (tokens[mainop].type) {
          case '!': res =  !res; break;
          case TK_MINUS: res =  -res; break;
          case TK_POINTER: res =  isa_vaddr_read(res, 4); break;
          default: *success = 0;
        }
        printf("[%d %d] %d \n", start, end, res);
        return res;
      }
    } else if (*success) {
      int res1 = eval(start, mainop - 1, success), res2 = eval(mainop + 1, end, success);
      switch (tokens[mainop].type) {
        case '+': res = res1 + res2; break;
        case '-': res = res1 - res2; break;
        case '*': res = res1 * res2; break;
        case '/': res = res1 / res2; break;
        case '&': res = res1 && res2; break;
        case '|': res = res1 || res2; break;
        case TK_EQ: res = res1 == res2; break;
        case TK_UEQ: res = res1 != res2; break;
        default: *success = 0;
      }
      printf("[%d %d] %d %d\n", start, end, res1, res2);
      return res;
    }
    return 0;
  }
  return 0;
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
