#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lexer.h"
#include "ast.h"
#include "parser.h"

extern char  yytext[];
extern FILE *yyin;
extern char *yyfile;
extern int   yycc, yyline;

int main (int argc, char *argv[]) {
  Node *ast;

  yyfile = argv[1];
  yyin   = fopen (argv[1], "r");
  if (NULL == yyin) {
    fprintf (stderr, "impossible d'ouvrir le fichier %s\n", argv[1]);
    exit (1);
  }

  next_token ();
  ast = rule_program ();

  Node_toDot   (ast);
  Node_destroy (ast);

  fclose (yyin);

  return 0;
}
