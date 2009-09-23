#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"

extern char  yytext[];
extern FILE *yyin;
extern int   yycc;

int main (int argc, char **argv) {
  yyin = fopen (argv[1], "r");
  if (NULL == yyin) {
    fprintf (stderr, "impossible d'ouvrir le fichier %s\n", argv[1]);
    exit (1);
  }

  next_token ();

  rule_arithmeticExpr ();

  fclose (yyin);

  return 0;
}
