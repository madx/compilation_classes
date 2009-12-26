#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include "symbol.h"

extern char  yytext[];
extern FILE *yyin;
extern char *yyfile;
extern int   yycc, yyline;
extern SymTable *yysym;

#define opt(s) (!(strcmp(argv[i], s)))

int main (int argc, char *argv[]) {
  Node *ast;
  enum { DOT, SYM } mode = DOT ;
  int i;

  for(i = 1; i < argc; i++) {
    if      (opt("-d")) mode = DOT;
    else if (opt("-s")) mode = SYM;
    else if (opt("-h")) {
      puts ("-d: produce dot");
      puts ("-s: produce symbol table");
      exit (EXIT_SUCCESS);
    } else {
      yyfile = argv[i];
    }
  }

  yyin = fopen (yyfile, "r");
  if (NULL == yyin) {
    fprintf (stderr, "unable to open %s\n", argv[1]);
    exit (EXIT_FAILURE);
  }

  next_token ();
  ast = rule_program ();

  switch (mode) {
  case DOT: AST_toDot (ast); break;
  case SYM:
    yysym = SymTable_new (
      Node_countType (ast, N_VAR_DEC) +
      Node_countType (ast, N_FUN_DEC) +
      Node_countType (ast, N_ARR_DEC)
    );

    SymTable_build (yysym, ast);

    SymTable_print   (yysym);
    SymTable_destroy (yysym);
    break;
  }

  Node_destroy (ast);
  fclose (yyin);

  return 0;
}
