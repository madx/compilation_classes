#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"

extern char  yytext[128];
extern FILE *yyin;
       int   yycc;
int depth = 0;

/* arithmeticExpr -> term '+' arithmeticExpr
 * arithmeticExpr -> term '-' arithmeticExpr
 * arithmeticExpr -> term
 */
void rule_arithmeticExpr () {
  if (yycc == NUMBER) {
    entering ("arithmetic expression");
    rule_term ();
    if (yycc == '+' || yycc == '-') {
      next_token ();
      rule_arithmeticExpr ();
    }
    exiting ("arithmetic expression");
  } else expecting ("number");
}

/* term -> factor '/' term
 * term -> factor '*' term
 * term -> factor
 */
void rule_term () {
  if (yycc == NUMBER) {
    entering ("term");
    rule_factor ();
    if (yycc == '*' || yycc == '/') {
      next_token ();
      rule_term ();
    }
    exiting ("term");
  } else expecting ("number");
}

/* factor -> NUMBER
 */
void rule_factor () {
  if (yycc == NUMBER) {
    entering ("factor");
    next_token ();
    exiting ("factor");
  } else expecting ("number");
}

/* Utilitaires */

void entering (char *s) {
  int i;
  for (i = 0; i < depth; i++) printf ("  ");
  printf ("entering %s\n", s);
  depth++;
}

void exiting (char *s) {
  int i;
  depth--;
  for (i = 0; i < depth; i++) printf ("  ");
  printf ("exiting %s\n", s);
}

void expecting (char *expected) {
  char type[64];
  switch (yycc) {
    case 330: sprintf(type, "variable identifier (%s)", yytext); break;
    case 331: sprintf(type, "function identifier (%s)", yytext); break;
    case 332: sprintf(type, "number (%s)", yytext);              break;
    default:  sprintf(type, "%s", yytext);                       break;
  }
  fprintf (stderr, "error: unexpected %s, expecting %s\n", type, expected);
  exit (EXIT_FAILURE);
}

void next_token () {
  yycc = yylex ();
  printf ("%d is %s\n", yycc, yytext);
}
