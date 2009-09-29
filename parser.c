#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"

extern char  yytext[128];
extern FILE *yyin;
extern int   yyline;
       int   yycc;
       int   depth = 0;
       char *yyfile;

/* program -> funDefList '.'
 * program -> varDecList ';' funDefList '.'
 */
void rule_program () {
  entering ("program");
  if (yycc == INT) {
    rule_varDecList ();

    if (yycc == ';') {
      next_token ();

    } else expecting ("';'");
  }

  if (yycc == FUN_ID) {
    rule_funDefList ();
  }

  if (yycc != '.') expecting ("'.'");
  exiting ("program");
}

/* varDecList -> varDec varDecList2
 */
void rule_varDecList () {
  entering ("var_dec_list");
  if (yycc == INT) {
    rule_varDec ();

    if (yycc == ',') {
      rule_varDecList2 ();
    }

  } else expecting ("keyword ENTIER");
  exiting ("var_dec_list");
}

/* varDecList2 -> ',' varDec varDecList2
 * varDecList2 -> void
 */
void rule_varDecList2 () {
  entering ("var_dec_list2");
  if (yycc == ',') {
    next_token ();

    if (yycc == INT) {
      rule_varDec ();

      if (yycc == ',') {
        rule_varDecList2 ();
      }
    } else expecting ("keyword ENTIER");
  } else expecting ("','");
  exiting ("var_dec_list2");
}

/* varDec -> INT VAR_ID
 * varDec -> INT VAR_ID '[' NUMBER ']'
 */
void rule_varDec () {
  entering ("var_dec");
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

          } else expecting ("']'");
        } else expecting ("number");
      }

    } else expecting ("variable identifier");
  } else expecting ("keyword ENTIER");
  exiting ("var_dec");
}

/* funDefList -> funDef funDefList
 * funDefList -> void
 */
void rule_funDefList () {
  entering ("fun_def_list");
  if (yycc == FUN_ID) {
    rule_funDef ();

    if (yycc == FUN_ID) {
      rule_funDefList ();
    }
  } else expecting ("function identifier");
  exiting ("fun_def_list");
}

/* funDef -> FUN_ID paramList instructionBlock
 * funDef -> FUN_ID paramList varDecList ';' instructionBlock
 */
void rule_funDef () {
  entering ("fun_def");
  if (yycc == FUN_ID) {
    next_token ();

    if (yycc == '(') {
      rule_paramList ();

      if (yycc == INT) {
        rule_varDecList ();

        if (yycc == ';') {
          next_token ();
        } else expecting ("';'");
      }

      if (yycc == '{') {
        rule_instructionBlock ();

      } else expecting ("'{'");
    } else expecting ("'('");
  } else expecting ("function identifier");
  exiting ("fun_def");
}

/* paramList -> '(' varDecList ')'
 * paramList -> '(' ')'
 */
void rule_paramList () {
  entering ("param_list");
  if (yycc == '(') {
    next_token ();
    if (yycc == INT) {
      rule_varDecList ();
    }
    if (yycc == ')') {
      next_token ();
    } else expecting ("')'");
  } else expecting ("'('");
  exiting ("param_list");
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
  entering ("instruction");
  if        (yycc == VAR_ID) { rule_setInstruction ();
  } else if (yycc == FUN_ID) { rule_callInstruction ();
  } else if (yycc == IF)     { rule_ifInstruction ();
  } else if (yycc == WHILE)  { rule_whileInstruction ();
  } else if (yycc == RETURN) { rule_returnInstruction ();
  } else if (yycc == WRITE)  { rule_writeInstruction ();
  } else if (yycc == ';')    { rule_voidInstruction ();
  } else if (yycc == '{')    { rule_instructionBlock ();
  } else expecting ("instruction");
  exiting ("instruction");
}

/* instructionBlock -> '{' instructionList '}'
 */
void rule_instructionBlock () {
  entering ("instruction_block");
  if (yycc == '{') {
    next_token ();
    rule_instructionList ();

    if (yycc == '}') {
      next_token ();

    } else expecting ("'}");
  } else expecting ("'{'");
  exiting ("instruction_block");
}

/* instructionList -> instruction instructionList
 * instructionList -> void
 */
void rule_instructionList () {
  entering ("instruction_list");
  if (yycc == VAR_ID || yycc == FUN_ID || yycc == '{'    || yycc == ';' ||
      yycc == IF     || yycc == WHILE  || yycc == RETURN || yycc == WRITE ) {
    rule_instruction ();

    if (yycc == VAR_ID || yycc == FUN_ID || yycc == '{'    || yycc == ';' ||
        yycc == IF     || yycc == WHILE  || yycc == RETURN || yycc == WRITE ) {
      rule_instructionList ();
    }

  } else if (yycc == '}') return;
  else expecting ("'}'");
  exiting ("instruction_list");
}

/* callInstruction -> funCall ';'
 */
void rule_callInstruction () {
  entering ("call_instruction");
  if (yycc == FUN_ID) {
    rule_funCall ();

    if (yycc == ';') {
      next_token ();

    } else expecting ("';'");
  } else expecting ("function identifier");
  exiting ("call_instruction");
}

/* setInstruction -> variable '=' expression ';'
 */
void rule_setInstruction () {
  entering ("set_instruction");
  if (yycc == VAR_ID) {
    rule_variable ();

    if (yycc == '=') {
      next_token ();
      rule_expression ();

      if (yycc == ';') {
        next_token ();

      } else expecting ("';'");
    } else expecting ("'='");
  } else expecting ("variable identifier");
  exiting ("set_instruction");
}

/* ifInstruction -> IF expression THEN instruction ELSE instruction
 * ifInstruction -> IF expression THEN instruction
 */
void rule_ifInstruction () {
  entering ("if_instruction");
  if (yycc == IF) {
    next_token ();

    if (yycc == '('  || yycc == NUMBER || yycc == VAR_ID ||
        yycc == READ || yycc == FUN_ID) {
      rule_expression ();

      if (yycc == THEN) {
        next_token ();

        if (yycc == VAR_ID  || yycc == FUN_ID || yycc == '{'    ||
            yycc == ';'     || yycc == IF     || yycc == WHILE  ||
            yycc == RETURN  || yycc == WRITE ) {
          rule_instruction ();

          if (yycc == ELSE) {
            next_token ();

            if (yycc == VAR_ID  || yycc == FUN_ID || yycc == '{'    ||
                yycc == ';'     || yycc == IF     || yycc == WHILE  ||
                yycc == RETURN || yycc == WRITE ) {
              rule_instruction ();
            } else expecting ("instruction");
          }

        } else expecting ("instruction");
      } else expecting ("keyword THEN");
    } else expecting ("expression");
  } else expecting ("keyword IF");
  exiting ("if_instruction");
}

/* whileInstruction -> WHILE expression DO instruction
 */
void rule_whileInstruction () {
  entering ("while_instruction");
  if (yycc == WHILE) {
    next_token ();

    if (yycc == '('  || yycc == NUMBER || yycc == VAR_ID ||
        yycc == READ || yycc == FUN_ID) {
      rule_expression ();

      if (yycc == DO) {
        next_token ();

        if (yycc == VAR_ID  || yycc == FUN_ID || yycc == '{'    ||
            yycc == ';'     || yycc == IF     || yycc == WHILE  ||
            yycc == RETURN || yycc == WRITE ) {
          rule_instruction ();

        } else expecting ("instruction");
      } else expecting ("keyword THEN");
    } else expecting ("expression");
  } else expecting ("keyword IF");
  exiting ("while_instruction");
}

/* returnInstruction -> RETURN expression ';'
 */
void rule_returnInstruction () {
  entering ("return_instruction");
  if (yycc == RETURN) {
    next_token ();

    if (yycc == '(' || yycc == NUMBER || yycc == VAR_ID ||
        yycc == READ || yycc == FUN_ID) {
      rule_expression ();

      if (yycc == ';') {
        next_token ();

      } else expecting ("';'");
    } else expecting ("expression");
  } else expecting ("keyword RETURN");
  exiting ("return_instruction");
}

/* writeInstruction -> WRITE '(' expression ')' ';'
 */
void rule_writeInstruction () {
  entering ("write_instruction");
  if (yycc == WRITE) {
    next_token ();

    if (yycc == '(') {
      next_token ();

      if (yycc == '(' || yycc == NUMBER || yycc == VAR_ID ||
          yycc == READ || yycc == FUN_ID) {
        rule_expression ();

        if (yycc == ')') {
          next_token ();

          if (yycc == ';') {
            next_token ();

          } else expecting ("';'");
        } else expecting ("')'");
      } else expecting ("expression");
    } else expecting ("'('");
  } else expecting ("keyword RETURN");
  exiting ("write_instruction");
}

/* voidInstruction -> ';'
 */
void rule_voidInstruction () {
  entering ("void_instruction");
  if (yycc == ';') next_token (); else expecting ("';'");
  exiting ("void_instruction");
}

/* expression -> conjunction OR expression
 * expression -> conjunction
 */
void rule_expression () {
  entering ("expression");
  if (yycc == '('  || yycc == NUMBER || yycc == VAR_ID ||
      yycc == READ || yycc == FUN_ID) {
    rule_conjunction ();

    if (yycc == OR) {
      next_token ();

      if (yycc == '('  || yycc == NUMBER || yycc == VAR_ID ||
          yycc == READ || yycc == FUN_ID) {
        rule_expression ();
      } else expecting ("expression after OR");
    }

  } else expecting ("conjunction");
  exiting ("expression");
}

/* conjunction -> comparison AND conjunction
 * conjunction -> comparison
 */
void rule_conjunction () {
  entering ("conjunction");
  if (yycc == '(' || yycc == NUMBER || yycc == VAR_ID ||
      yycc == READ || yycc == FUN_ID) {
    rule_comparison ();

    if (yycc == AND) {
      next_token ();

      if (yycc == '(' || yycc == NUMBER || yycc == VAR_ID ||
          yycc == READ || yycc == FUN_ID) {
        rule_conjunction ();
      } else expecting ("expression after AND");
    }

  } else expecting ("comparison");
  exiting ("conjunction");
}

/* comparison -> arithmeticExpr EQ arithmeticExpr
 * comparison -> arithmeticExpr NEQ arithmeticExpr
 * comparison -> arithmeticExpr '<' arithmeticExpr
 * comparison -> arithmeticExpr LE arithmeticExpr
 * comparison -> arithmeticExpr
 */
void rule_comparison () {
  entering ("comparison");
  if (yycc == '('  || yycc == NUMBER || yycc == VAR_ID ||
      yycc == READ || yycc == FUN_ID) {
    rule_arithmeticExpr ();

    if (yycc == EQ || yycc == NEQ || yycc == '<' || yycc == LE) {
      next_token ();

      if (yycc == '('  || yycc == NUMBER || yycc == VAR_ID ||
          yycc == READ || yycc == FUN_ID) {
        rule_arithmeticExpr ();
      } else expecting ("arithmetic expression");
    }

  } else expecting ("arithmetic expression");
  exiting ("comparison");
}

/* arithmeticExpr -> term '+' arithmeticExpr
 * arithmeticExpr -> term '-' arithmeticExpr
 * arithmeticExpr -> term
 */
void rule_arithmeticExpr () {
  entering ("arithmetic_expr");
  if (yycc == '('  || yycc == NUMBER || yycc == VAR_ID ||
      yycc == READ || yycc == FUN_ID) {
    rule_term ();

    if (yycc == '+' || yycc == '-') {
      next_token ();
      rule_arithmeticExpr ();
    }

  } else expecting ("term");
  exiting ("arithmetic_expr");
}

/* term -> factor '/' term
 * term -> factor '*' term
 * term -> factor
 */
void rule_term () {
  entering ("term");
  if (yycc == '('  || yycc == NUMBER || yycc == VAR_ID ||
      yycc == READ || yycc == FUN_ID) {
    rule_factor ();

    if (yycc == '/' || yycc == '*') {
      next_token ();
      rule_term ();
    }

  } else expecting ("factor");
  exiting ("term");
}

/* factor -> '(' expression ')'
 * factor -> NUMBER
 * factor -> funCall
 * factor -> variable
 * factor -> READ '(' ')'
 */
void rule_factor () {
  entering ("factor");
  if (yycc == NUMBER) {
    next_token ();

  } else if (yycc == VAR_ID) {
    rule_variable ();

  } else if (yycc == FUN_ID) {
    rule_funCall ();

  } else if (yycc == '(') {
    next_token ();
    rule_expression ();

    if (yycc == ')') {
      next_token ();

    } else expecting ("')'");

  } else if (yycc == READ) {
    next_token ();

    if (yycc == '(') {
      next_token ();

      if (yycc == ')') {
        next_token ();

      } else expecting ("')'");
    } else expecting ("'('");

  } else expecting ("factor");
  exiting ("factor");
}

/* variable -> VAR_ID '[' expression ']'
 * variable -> VAR_ID
 */
void rule_variable () {
  entering ("variable");
  if (yycc == VAR_ID) {
    next_token ();

    if (yycc == '[') {
      next_token ();
      rule_expression ();

      if (yycc == ']') {
        next_token ();
      } else expecting ("']'");
    }

  } else expecting ("variable identifier");
  exiting ("variable");
}

/* funCall -> FUN_ID arguments
 */
void rule_funCall () {
  entering ("fun_call");
  if (yycc == FUN_ID) {
    next_token ();
    rule_arguments ();

  } else expecting ("function identifier");
  exiting ("fun_call");
}

/* arguments -> '(' exprList ')'
 */
void rule_arguments () {
  entering ("arguments");
  if (yycc == '(') {
    next_token ();
    rule_exprList ();

    if (yycc == ')') {
      next_token ();
    } else expecting ("')'");
  } else expecting ("'('");
  exiting ("arguments");
}

/* exprList -> expression exprList2
 * exprList -> void
 */
void rule_exprList () {
  entering ("expr_list");
  if (yycc == '('  || yycc == NUMBER || yycc == VAR_ID ||
      yycc == READ || yycc == FUN_ID) {
    rule_expression ();

    if (yycc == ',') {
      rule_exprList2 ();
    }
  }
  exiting ("expr_list");
}

/* exprList2 -> ',' expression exprList2
 * exprList2 -> void
 */
void rule_exprList2 () {
  entering ("expr_list2");
  if (yycc == ',') {
    next_token ();

    if (yycc == '('  || yycc == NUMBER || yycc == VAR_ID ||
      yycc == READ || yycc == FUN_ID) {
      rule_expression ();

      if (yycc == '('  || yycc == NUMBER || yycc == VAR_ID ||
          yycc == READ || yycc == FUN_ID) {
        rule_exprList2 ();
      }

    } else expecting ("expression");
  } else expecting ("','");
  exiting ("expr_list2");
}

/* Utilitaires */

void entering (char *rule) {
  /* int i; */
  /* for (i = 0; i < depth; i++) printf ("  "); */
  /* printf ("<%s>\n", rule); */
  /* depth++; */
}

void exiting (char *rule) {
  /* int i; */
  /* depth--; */
  /* for (i = 0; i < depth; i++) printf ("  "); */
  /* printf ("</%s>\n", rule); */
}

void expecting (char *expected) {
  char type[280];
  switch (yycc) {
    case 330: sprintf(type, "variable identifier (%s)", yytext); break;
    case 331: sprintf(type, "function identifier (%s)", yytext); break;
    case 332: sprintf(type, "number (%s)", yytext);              break;
    case 0:   sprintf(type, "end of file");                      break;
    default:  sprintf(type, "'%s'", yytext);                     break;
  }
  fprintf (stderr, "%s.%d: error: unexpected %s, expecting %s\n",
           yyfile, yyline, type, expected);
  exit (EXIT_FAILURE);
}

void next_token () {
  yycc = yylex ();
  printf ("%3d -> %s\n", yycc, yytext);
}
