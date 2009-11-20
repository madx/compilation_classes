#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lexer.h"

extern char yytext[];
extern FILE *yyin;

int main (int argc, char **argv) {
  int uniteCourante;
  yyin = fopen (argv[1], "r");
  if (NULL == yyin) {
    fprintf (stderr, "impossible d'ouvrir le fichier %s\n", argv[1]);
    exit (1);
  }
  uniteCourante = yylex ();
  while (uniteCourante != 0) {
    printf ("%s\t%d\n", yytext, uniteCourante);
    uniteCourante = yylex ();
  }

  fclose (yyin);

  return 0;
}
