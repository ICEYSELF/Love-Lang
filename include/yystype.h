#ifndef YYSTYPE_H
#define YYSTYPE_H

#include "ast.h"
#include "jjvalue.h"

typedef struct {
  jjvalue_t val;
  unsigned row : 16;
  unsigned col : 16;
  unsigned replaced : 1;
  unsigned token_kind : 15;
  unsigned : 16;
} token_t;

typedef union {
  token_t token;
  ast_node_base_t *ast;
} yystype_t;

#define YYSTYPE yystype_t

#endif
