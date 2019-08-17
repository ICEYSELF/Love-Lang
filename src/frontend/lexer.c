#include "mstring.h"
#include "util.h"
#include "yystype.h"
#include "y.tab.h"

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static bool my_strcmpi(const char *s1, const char *s2) {
  while (*s1 != '\0' && *s2 != '\0') {
    if (tolower(*s1) != tolower(*s2)) {
      return false;
    }
    ++s1;
    ++s2;
  }
  if (*s1 != '\0' || *s2 != '\0') {
    return false;
  }
  return true;
}

FILE *fp_lex_in;
bool repl_mode = false;
bool lexer_error = false;

static int peeked_char_ = '\0';

static char curchar_ = '\xff';
static uint16_t currow_ = 1;
static uint16_t curcol_ = 0;

static void peek_one_char(void) {
  assert(peeked_char_ == '\0' && "Already having one peeked char");
  int value = fgetc(fp_lex_in);
  peeked_char_ = value != EOF ? (char)value : '\0';
}

static void get_next_char(void) {
  if (peeked_char_ != '\0') {
    curchar_ = peeked_char_;
    peeked_char_ = '\0';
  }
  else {
    int value = fgetc(fp_lex_in);
    curchar_ = value != EOF ? (char)value : '\0';
  }

  if (curchar_ == '\n') {
    currow_++;
    curcol_ = 0;
  }
  else {
    curcol_++;
  }
}

static char curchar(void) {
  return curchar_;
}

static uint16_t currow(void) {
  return currow_;
}

static uint16_t curcol(void) {
  return curcol_;
}

static char peeked_char(void) {
  return peeked_char_;
}

static void lex_warn(const char *warn_text, uint16_t line,
                     uint16_t col) {
  fprintf(stderr, "at (%d, %d): warning: %s\n", line, col, warn_text);
  if (repl_mode) {
    fprintf(stderr, "in REPL mode, clearing input\n");
    while (curchar() != '\n' && curchar() != '\0') {
      get_next_char();
    }
  }
  if (curchar() == '\n') curchar_ = '\xff';
  lexer_error = true;
}

static int maybe_id_to_kwd(const char *str) {
  #define STRING_CASE(EXPECT, TOKEN_KIND) \
    if (my_strcmpi(str, EXPECT)) { \
      yylval.token.token_kind = TOKEN_KIND; \
      return TOKEN_KIND; \
    }

  STRING_CASE("function", TK_FUNCTION)
  STRING_CASE("takes", TK_TAKES)
  STRING_CASE("returns", TK_RETURNS)
  STRING_CASE("begin", TK_BEGIN)
  STRING_CASE("end", TK_END)
  STRING_CASE("if", TK_IF)
  STRING_CASE("then", TK_THEN)
  STRING_CASE("else", TK_ELSE)
  STRING_CASE("while", TK_WHILE)
  STRING_CASE("for", TK_FOR)
  STRING_CASE("to", TK_TO)
  STRING_CASE("return", TK_RETURN)
  STRING_CASE("true", TK_TRUE)
  STRING_CASE("false", TK_FALSE)

  STRING_CASE("plus", TK_ESYM_PLUS)
  STRING_CASE("add", TK_ESYM_PLUS)
  STRING_CASE("minus", TK_ESYM_MINUS)
  STRING_CASE("mult", TK_ESYM_ASTER)
  STRING_CASE("multi", TK_ESYM_ASTER)
  STRING_CASE("multiply", TK_ESYM_ASTER)
  STRING_CASE("times", TK_ESYM_ASTER)
  STRING_CASE("aster", TK_ESYM_ASTER)
  STRING_CASE("asterisk", TK_ESYM_ASTER)
  STRING_CASE("div", TK_ESYM_SLASH)
  STRING_CASE("divide", TK_ESYM_SLASH)
  STRING_CASE("slash", TK_ESYM_SLASH)
  STRING_CASE("eqeq", TK_ESYM_EQEQ)
  STRING_CASE("equals", TK_ESYM_EQEQ)
  STRING_CASE("eq", TK_ESYM_EQEQ)
  STRING_CASE("assign", TK_ESYM_EQ)
  STRING_CASE("perc", TK_ESYM_PERCENT)
  STRING_CASE("percent", TK_ESYM_PERCENT)
  STRING_CASE("lbrace", TK_ESYM_LBRACE)
  STRING_CASE("rbrace", TK_ESYM_RBRACE)
  STRING_CASE("and", TK_ESYM_AMPAMP)
  STRING_CASE("or", TK_ESYM_PIPE)
  STRING_CASE("lt", TK_ESYM_LT)
  STRING_CASE("gt", TK_ESYM_GT)
  STRING_CASE("leq", TK_ESYM_LEQ)
  STRING_CASE("geq", TK_ESYM_GEQ)
  STRING_CASE("neq", TK_ESYM_NEQ)

  #undef STRING_CASE

  return TK_ID;
}

static int lex_id_or_kwd(void) {
  char buffer[32];
  uint16_t row = currow();
  uint16_t col = curcol();
  int idx = 0;
  while (idx < 31 && isalnum(curchar())) {
    buffer[idx] = curchar();
    ++idx;
    get_next_char();
  }
  buffer[idx] = '\0';
  if (idx == 31) {
    lex_warn("Identifier length exceeds 31 characters", row, col);
    while (isalnum(curchar())) {
      get_next_char();
    }
  }
  yylval.token.row = row;
  yylval.token.col = row;
  yylval.token.replaced = 0;

  int convert_result = maybe_id_to_kwd(buffer);
  if (convert_result != TK_ID) {
    return convert_result;
  }
  yylval.token.token_kind = TK_ID;
  yylval.token.val.svalue = create_string(buffer);
  return TK_ID;
}

static int lex_number(void) {
  yylval.token.replaced = 0;
  yylval.token.row = currow();
  yylval.token.col = curcol();
  int64_t int_part = 0;
  while (isdigit(curchar())) {
    int_part *= 10;
    int_part += curchar() - '0';
    get_next_char();
  }

  if (curchar() == '.') {
    get_next_char();
    double float_value = int_part;
    double temp = 0;
    while (isdigit(curchar())) {
      temp *= 10;
      temp += curchar() - '0';
      get_next_char();
    }
    while (temp > 1) {
      temp /= 10;
    }
    float_value += temp;
    yylval.token.token_kind = TK_NUM_FLOAT;
    yylval.token.val.fvalue = float_value;
    return TK_NUM_FLOAT;
  }

  yylval.token.token_kind = TK_NUM_INT;
  yylval.token.val.ivalue = int_part;
  return TK_NUM_INT;
}

static int lex_common_sym(void) {
  yylval.token.replaced = 0;
  yylval.token.row = currow();
  yylval.token.col = curcol();
  switch(curchar()) {
  case '.': yylval.token.token_kind = TK_SYM_DOT;      break;
  case ',': yylval.token.token_kind = TK_SYM_COMMA;    break;
  case ';': yylval.token.token_kind = TK_SYM_SEMI;     break;
  case '[': yylval.token.token_kind = TK_SYM_LBRACKET; break;
  case ']': yylval.token.token_kind = TK_SYM_RBRACKET; break;
  case '(': yylval.token.token_kind = TK_ESYM_LPAREN;  break;
  case ')': yylval.token.token_kind = TK_ESYM_RPAREN;  break;
  case '{': yylval.token.token_kind = TK_ESYM_LBRACE;  break;
  case '}': yylval.token.token_kind = TK_ESYM_RBRACE;  break;
  case '+': yylval.token.token_kind = TK_ESYM_PLUS;    break;
  case '-': yylval.token.token_kind = TK_ESYM_MINUS;   break;
  case '*': yylval.token.token_kind = TK_ESYM_ASTER;   break;
  case '/': yylval.token.token_kind = TK_ESYM_SLASH;   break;
  case '%': yylval.token.token_kind = TK_ESYM_PERCENT; break;
  case '^': yylval.token.token_kind = TK_ESYM_CARET;   break;

  case '=': {
    peek_one_char();
    if (peeked_char() == '=') {
      get_next_char();
      yylval.token.token_kind = TK_ESYM_EQEQ;
    }
    else {
      yylval.token.token_kind = TK_ESYM_EQ;
    }
    break;
  }

  case '<': {
    peek_one_char();
    if (peeked_char() == '=') {
      get_next_char();
      yylval.token.token_kind = TK_ESYM_LEQ;
    }
    else {
      yylval.token.token_kind = TK_ESYM_LT;
    }
    break;
  }

  case '>': {
    peek_one_char();
    if (peeked_char() == '=') {
      get_next_char();
      yylval.token.token_kind = TK_ESYM_GEQ;
    }
    else {
      yylval.token.token_kind = TK_ESYM_GT;
    }
    break;
  }

  case '!': {
    peek_one_char();
    if (peeked_char() == '=') {
      get_next_char();
      yylval.token.token_kind = TK_ESYM_NEQ;
    }
    else {
      lex_warn("! does not values anything, replace that with -\n",
               currow(), curcol());
      yylval.token.token_kind = TK_ESYM_MINUS;
    }
    break;
  }

  case '&': {
    peek_one_char();
    if (peeked_char() == '&') {
      get_next_char();
      yylval.token.token_kind = TK_ESYM_AMPAMP;
    }
    else {
      yylval.token.token_kind = TK_ESYM_AMP;
    }
    break;
  }

  case '|': {
    peek_one_char();
    if (peeked_char() == '|') {
      get_next_char();
      yylval.token.token_kind = TK_ESYM_PIPEPIPE;
    }
    else {
      yylval.token.token_kind = TK_ESYM_PIPE;
    }
  }

  }
  get_next_char();
  return yylval.token.token_kind;
}

static int lex_string(void) {
  yylval.token.token_kind = TK_STR;
  yylval.token.row = currow();
  yylval.token.col = curcol();
  yylval.token.replaced = 0;

  // TODO: Shall we make out a mutable string?
  size_t cap = 64;
  size_t size = 0;
  char *buffer = NEWN(char, 64);
  get_next_char();
  while (curchar() != '\'' && curchar() != '\0') {
    if (cap == size - 1) {
      char *new_buffer = NEWN(char, cap * 2);
      strncpy(new_buffer, buffer, cap);
      cap *= 2;
      free(buffer);
      buffer = new_buffer;
    }

    if (curchar() == '\\') {
      peek_one_char();
      switch (peeked_char()) {
      case '\\': get_next_char(); buffer[size] = '\\'; break;
      case 'n':  get_next_char(); buffer[size] = '\n'; break;
      case 't':  get_next_char(); buffer[size] = '\t'; break;
      case 'f':  get_next_char(); buffer[size] = '\f'; break;
      case 'v':  get_next_char(); buffer[size] = '\v'; break;
      case '\'': get_next_char(); buffer[size] = '\''; break;
      default:
        lex_warn("Invalid conversion sequence, ignoring",
                 currow(), curcol());
      }
    }
    else {
      buffer[size] = curchar();
    }
    ++size;
    get_next_char();
  }

  if (curchar() == '\0') {
    lex_warn("Unclosed string, ignoring", currow(), curcol());
  }
  else {
    get_next_char();
  }
  buffer[size] = '\0';

  yylval.token.val.svalue = create_string(buffer);
  free(buffer);
  return yylval.token.token_kind;
}

int yylex(void) {
  if (curchar() == '\xff') {
    get_next_char();
  }

  while (1) {
    switch (curchar()) {
    case '\0':
      return -1;

    case '.':
      peek_one_char();
      if (peeked_char() == '.') {
        get_next_char();
        get_next_char();
        return -1;
      }
      else {
        ungetc(peeked_char(), fp_lex_in);
        peeked_char_ = '\0';
        return lex_common_sym();
      }

    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
    case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
    case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
    case 's': case 't': case 'u': case 'v': case 'w': case 'x':
    case 'y': case 'z': case 'A': case 'B': case 'C': case 'D':
    case 'E': case 'F': case 'G': case 'H': case 'I': case 'J':
    case 'K': case 'L': case 'M': case 'N': case 'O': case 'P':
    case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V':
    case 'W': case 'X': case 'Y': case 'Z':
      return lex_id_or_kwd();

    case '+': case '-': case '*': case '/': case '%': case '[':
    case ']': case '=': case '<': case '>': case '&': case '|':
    case '^': case '!': case '(': case ')': case ',': case ';':
      return lex_common_sym();

    case '1': case '2': case '3': case '4': case '5': case '6':
    case '7': case '8': case '9': case '0':
      return lex_number();

    case '\'':
      return lex_string();

    case ' ': case '\n': case '\t': case '\v': case '\f':
      get_next_char();
      break;

    default:
      lex_warn("Unknown char, skipping", currow(), curcol());
      if (repl_mode) {
        return -1;
      }
      get_next_char();
    }
  }
}
