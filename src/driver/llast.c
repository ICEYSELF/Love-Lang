#include "ast.h"
#include "config.h"
#include "tree_dump.h"
#include "y.tab.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  fprintf(stderr, "AST printer for %s -- %s\n",
          BAS_LANGUAGE_NAME, BAS_DISCRIPTION);
  fprintf(stderr, "Created by %s, licensed under %s\n",
          BAS_AUTHOR, BAS_LICENSE);
  fprintf(stderr, "  Frontend version %d.%d.%d\n",
          VER_PLCFRONT_MAJOR, VER_PLCFRONT_MINOR, VER_PLCFRONT_REVISE);
  fputc('\n', stderr);

  extern FILE *fp_lex_in;
  extern ast_node_base_t *glob_ast;
  int yyparse(void);

  if (argc == 2) {
    fp_lex_in = fopen(argv[1], "r");
    if (fp_lex_in == NULL) {
      fprintf(stderr, "Cannot open file %s\n", argv[1]);
      exit(-1);
    }
  }
  else if (argc == 1) {
    fp_lex_in = stdin;
  }
  else {
    fprintf(stderr, "Too many arguments\n");
  }

  yyparse();
  if (fp_lex_in != stdin) {
    fclose(fp_lex_in);
  }

  if (glob_ast != NULL) {
    tree_dump_pldot(glob_ast);
  }
  return 0;
}
