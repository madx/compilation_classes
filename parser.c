#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"

extern char  yytext[128];
extern FILE *yyin;
       int   yycc;
int depth = 0;

/* program -> funDefList '.'
 * program -> varDecList ';' funDefList '.'
 */
void rule_program () {
  if (yycc == INT) {
    rule_varDecList ();
    if (yycc == ';') {
      next_token ();
    } else expecting ("';'");
  }

  if (yycc == FUN_ID) {
    rule_funDefList ();
  } else expecting ("function declaration");

  if (yycc == '.') {
    puts ("program is terminated");
  } else expecting ("'.'");
}

/* varDecList -> varDec varDecList2
 */
void rule_varDecList () {
  if (yycc == INT) {
    rule_varDec ();
    if (yycc == ',') {
      rule_varDecList2 ();
    }
  } else expecting ("keyword ENTIER");
}

/* varDecList2 -> ',' varDec varDecList2
 * varDecList2 -> void
 */
void rule_varDecList2 () {
  if (yycc == ',') {
    next_token ();
    if (yycc == INT) {
      rule_varDec ();
      if (yycc == ',') {
        rule_varDecList2 ();
      }
    } else expecting ("keyword ENTIER");
  } else expecting ("','");
}

/* varDec -> INT VAR_ID
 * varDec -> INT VAR_ID '[' NUMBER ']'
 */
void rule_varDec () {
  if (yycc == INT) {
    next_token ();
    if (yycc == VAR_ID) {
      next_token ();

      if (yycc == '[') {
        next_token ();
        if (yycc == NUMBER) {
          next_token ();
          if (yycc == ']') {
            next_token ();
          } else expecting ("matching closing bracket (']')");
        } else expecting ("number");
      }

    } else expecting ("variable identifier");
  } else expecting ("keyword ENTIER");
}

/* funDefList -> funDef funDefList
 * funDefList -> void
 */
void rule_funDefList () {
}

/* funDef -> FUN_ID paramList instructionBlock
 * funDef -> FUN_ID paramList varDecList ';' instructionBlock
 */
void rule_funDef () {
}

/* paramList -> '(' varDecList ')'
 * paramList -> '(' ')'
 */
void rule_paramList () {
}

/* instruction -> instructionBlock
 * instruction -> callInstruction
 * instruction -> setInstruction
 * instruction -> ifInstruction
 * instruction -> whileInstruction
 * instruction -> returnInstruction
 * instruction -> writeInstruction
 * instruction -> voidInstruction
 */
void rule_instruction () {
}

/* instructionBlock -> '{' instructionList '}'
 */
void rule_instructionBlock () {
}

/* instructionList -> instruction instructionList
 * instructionList -> void
 */
void rule_instructionList () {
}

/* callInstruction -> funCall ';'
 */
void rule_callInstruction () {
}

/* setInstruction -> variable '=' expression ';'
 */
void rule_setInstruction () {
}

/* ifInstruction -> IF expression THEN instruction ELSE instruction
 * ifInstruction -> IF expression THEN instruction
 */
void rule_ifInstruction () {
}

/* whileInstruction -> WHILE expression DO instruction
 */
void rule_whileInstruction () {
}

/* returnInstruction -> RETURN expression ';'
 */
void rule_returnInstruction () {
}

/* writeInstruction -> WRITE '(' expression ')' ';'
 */
void rule_writeInstruction () {
}

/* voidInstruction -> ';'
 */
void rule_voidInstruction () {
}

/* expression -> conjunction OR expression
 * expression -> conjunction
 */
void rule_expression () {
}

/* conjunction -> comparison AND conjunction
 * conjunction -> comparison
 */
void rule_conjunction () {
}

/* comparison -> arithmeticExpr EQ arithmeticExpr
 * comparison -> arithmeticExpr NEQ arithmeticExpr
 * comparison -> arithmeticExpr '<' arithmeticExpr
 * comparison -> arithmeticExpr LE arithmeticExpr
 * comparison -> arithmeticExpr
 */
void rule_comparison () {
  if (yycc == NUMBER) {
    entering ("comparison");
    rule_arithmeticExpr ();
    if (yycc == EQ || yycc == NEQ || yycc == '<' || yycc == LE) {
      next_token ();
      rule_arithmeticExpr ();
    }
    exiting ("comparison");
  } else expecting ("number");
}

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
  printf ("%3d -> %s\n", yycc, yytext);
}
