#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "ast.h"
#include "parser.h"

extern char  *node_names[];
extern char  yytext[128];
extern FILE *yyin;
extern int   yyline;
       int   yycc;
       int   depth = 0;
       char *yyfile;

/* program -> funDefList '.'
 * program -> varDecList ';' funDefList '.'
 */
Node * rule_program () {
  Node *var_dec_list = NULL, *fun_def_list = NULL, *child = NULL;
  Value *val;
  if (yycc == INT) {
    var_dec_list = rule_varDecList ();

    if (yycc == ';') {
      next_token ();

    } else expecting ("';'");
  }

  if (yycc == FUN_ID) {
    fun_def_list = rule_funDefList ();
  }

  if (yycc != '.') expecting ("'.'");

  if (NULL != var_dec_list) {
    Node_lastSibling (var_dec_list)->next = fun_def_list;
    child = var_dec_list;
  } else {
    child = fun_def_list;
  }

  val = Value_str ("root");

  return Node_new (N_PROGRAM, val, NULL, child);
}

/* varDecList -> varDec varDecList2
 */
Node * rule_varDecList () {
  Node *head = NULL, *tail = NULL;

  if (yycc == INT) {
    head = rule_varDec ();

    if (yycc == ',') {
      tail = rule_varDecList2 ();
      Node_lastSibling(head)->next = tail;
    }

  } else expecting ("keyword ENTIER");

  return Node_new (N_DEC_LIST, NULL, NULL, head);
}

/* varDecList2 -> ',' varDec varDecList2
 * varDecList2 -> void
 */
Node * rule_varDecList2 () {
  Node *head = NULL, *tail = NULL;

  if (yycc == ',') {
    next_token ();

    if (yycc == INT) {
      head = rule_varDec ();

      if (yycc == ',') {
        tail = rule_varDecList2 ();
        Node_lastSibling(head)->next = tail;
      }
    } else expecting ("keyword ENTIER");
  } else expecting ("','");

  return Node_new (N_DEC_LIST, NULL, NULL, head);
}

/* varDec -> INT VAR_ID
 * varDec -> INT VAR_ID '[' NUMBER ']'
 */
Node * rule_varDec () {
  Value *name = NULL;
  Node  *index = NULL;

  if (yycc == INT) {
    next_token ();

    if (yycc == VAR_ID) {
      name = Value_str (yytext);
      next_token ();

      if (yycc == '[') {
        next_token ();

        if (yycc == NUMBER) {
          index = Node_new (N_INT_EXP,
                            Value_int (atoi (yytext)),
                            NULL, NULL);
          next_token ();

          if (yycc == ']') {
            next_token ();

          } else expecting ("']'");
        } else expecting ("number");
      }

    } else expecting ("variable identifier");
  } else expecting ("keyword ENTIER");

  return Node_new (N_VAR_DEC, name, index, NULL);
}

/* funDefList -> funDef funDefList
 * funDefList -> void
 */
Node * rule_funDefList () {
  Node *head = NULL, *tail = NULL;

  if (yycc == FUN_ID) {
    head = rule_funDef ();

    if (yycc == FUN_ID) {
      tail = rule_funDefList ();
      Node_lastSibling(head)->next = tail;
    }
  } else expecting ("function identifier");

  return Node_new (N_DEC_LIST, NULL, NULL, head);
}

/* funDef -> FUN_ID paramList instructionBlock
 * funDef -> FUN_ID paramList varDecList ';' instructionBlock
 */
Node * rule_funDef () {
  Value *name = NULL;
  Node  *params = NULL, *variables = NULL, *body = NULL;

  if (yycc == FUN_ID) {
    name = Value_str (yytext);
    next_token ();

    if (yycc == '(') {
      params = rule_paramList ();

      if (yycc == INT) {
        variables = rule_varDecList ();
        Node_lastSibling(params)->next = variables;

        if (yycc == ';') {
          next_token ();
        } else expecting ("';'");
      }

      if (yycc == '{') {
        body = rule_instructionBlock ();
        if (NULL != variables) { Node_lastSibling (variables)->next = body; }
        else                   { Node_lastSibling (params)->next    = body; }

      } else expecting ("'{'");
    } else expecting ("'('");
  } else expecting ("function identifier");

  return Node_new (N_FUN_DEC, name, NULL, params);
}

/* paramList -> '(' varDecList ')'
 * paramList -> '(' ')'
 */
Node * rule_paramList () {
  Node *list = NULL;

  if (yycc == '(') {
    next_token ();

    if (yycc == INT) {
      list = rule_varDecList ();
    }

    if (yycc == ')') {
      next_token ();

    } else expecting ("')'");
  } else expecting ("'('");

  return list;
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
Node * rule_instruction () {
  if (yycc == VAR_ID) {
    return rule_setInstruction ();

  } else if (yycc == FUN_ID) {
    return rule_callInstruction ();

  } else if (yycc == IF)     {
    return rule_ifInstruction ();

  } else if (yycc == WHILE)  {
    return rule_whileInstruction ();

  } else if (yycc == RETURN) {
    return rule_returnInstruction ();

  } else if (yycc == WRITE)  {
    return rule_writeInstruction ();

  } else if (yycc == ';')    {
    return rule_voidInstruction ();

  } else if (yycc == '{')    {
    return rule_instructionBlock ();

  } else expecting ("instruction");
  return NULL;
}

/* instructionBlock -> '{' instructionList '}'
 */
Node * rule_instructionBlock () {
  Node *list = NULL;

  if (yycc == '{') {
    next_token ();
    list = rule_instructionList ();

    if (yycc == '}') {
      next_token ();

    } else expecting ("'}");
  } else expecting ("'{'");

  return list;
}

/* instructionList -> instruction instructionList
 * instructionList -> void
 */
Node * rule_instructionList () {
  Node *head = NULL, *tail = NULL;

  if (yycc == VAR_ID || yycc == FUN_ID || yycc == '{'    || yycc == ';' ||
      yycc == IF     || yycc == WHILE  || yycc == RETURN || yycc == WRITE ) {
    head = rule_instruction ();

    if (yycc == VAR_ID || yycc == FUN_ID || yycc == '{'    || yycc == ';' ||
        yycc == IF     || yycc == WHILE  || yycc == RETURN || yycc == WRITE ) {
      tail = rule_instructionList ();
      Node_lastSibling(head)->next = tail;
    }

  }
  if (yycc != '}') expecting ("'}'");

  return Node_new (N_INST_LIST, NULL, NULL, head);
}

/* callInstruction -> funCall ';'
 */
Node * rule_callInstruction () {
  Node *call = NULL;

  if (yycc == FUN_ID) {
    call = rule_funCall ();

    if (yycc == ';') {
      next_token ();

    } else expecting ("';'");
  } else expecting ("function identifier");

  return call;
}

/* setInstruction -> variable '=' expression ';'
 */
Node * rule_setInstruction () {
  Node *var = NULL, *expr = NULL;

  if (yycc == VAR_ID) {
    var = rule_variable ();

    if (yycc == '=') {
      next_token ();
      expr = rule_expression ();
      Node_lastSibling (var)->next = expr;

      if (yycc == ';') {
        next_token ();

      } else expecting ("';'");
    } else expecting ("'='");
  } else expecting ("variable identifier");

  return Node_new (N_SET_INST, NULL, NULL, var);
}

/* ifInstruction -> IF expression THEN instruction ELSE instruction
 * ifInstruction -> IF expression THEN instruction
 */
Node * rule_ifInstruction () {
  Node *expr = NULL, *then_inst = NULL, *else_inst = NULL;

  if (yycc == IF) {
    next_token ();

    if (yycc == '('  || yycc == NUMBER || yycc == VAR_ID ||
        yycc == READ || yycc == FUN_ID) {
      expr = rule_expression ();

      if (yycc == THEN) {
        next_token ();

        if (yycc == VAR_ID  || yycc == FUN_ID || yycc == '{'    ||
            yycc == ';'     || yycc == IF     || yycc == WHILE  ||
            yycc == RETURN  || yycc == WRITE ) {
          then_inst = rule_instruction ();
          expr->next = then_inst;

          if (yycc == ELSE) {
            next_token ();

            if (yycc == VAR_ID  || yycc == FUN_ID || yycc == '{'    ||
                yycc == ';'     || yycc == IF     || yycc == WHILE  ||
                yycc == RETURN || yycc == WRITE ) {
              else_inst = rule_instruction ();
              then_inst->next = else_inst;
            } else expecting ("instruction");
          }

        } else expecting ("instruction");
      } else expecting ("keyword THEN");
    } else expecting ("expression");
  } else expecting ("keyword IF");

  return Node_new (N_IF_INST, NULL, NULL, expr);
}

/* whileInstruction -> WHILE expression DO instruction
 */
Node * rule_whileInstruction () {
  Node *expr = NULL, *inst = NULL;

  if (yycc == WHILE) {
    next_token ();

    if (yycc == '('  || yycc == NUMBER || yycc == VAR_ID ||
        yycc == READ || yycc == FUN_ID) {
      expr = rule_expression ();

      if (yycc == DO) {
        next_token ();

        if (yycc == VAR_ID  || yycc == FUN_ID || yycc == '{'    ||
            yycc == ';'     || yycc == IF     || yycc == WHILE  ||
            yycc == RETURN || yycc == WRITE ) {
          inst = rule_instruction ();
          Node_lastSibling (expr)->next = inst;

        } else expecting ("instruction");
      } else expecting ("keyword THEN");
    } else expecting ("expression");
  } else expecting ("keyword IF");

  return Node_new (N_WHILE_INST, NULL, NULL, expr);
}

/* returnInstruction -> RETURN expression ';'
 */
Node * rule_returnInstruction () {
  Node *expr = NULL;

  if (yycc == RETURN) {
    next_token ();

    if (yycc == '(' || yycc == NUMBER || yycc == VAR_ID ||
        yycc == READ || yycc == FUN_ID) {
      expr = rule_expression ();

      if (yycc == ';') {
        next_token ();

      } else expecting ("';'");
    } else expecting ("expression");
  } else expecting ("keyword RETURN");

  return Node_new (N_RETURN_INST, NULL, NULL, expr);
}

/* writeInstruction -> WRITE '(' expression ')' ';'
 */
Node * rule_writeInstruction () {
  Node *expr = NULL;

  if (yycc == WRITE) {
    next_token ();

    if (yycc == '(') {
      next_token ();

      if (yycc == '(' || yycc == NUMBER || yycc == VAR_ID ||
          yycc == READ || yycc == FUN_ID) {
        expr = rule_expression ();

        if (yycc == ')') {
          next_token ();

          if (yycc == ';') {
            next_token ();

          } else expecting ("';'");
        } else expecting ("')'");
      } else expecting ("expression");
    } else expecting ("'('");
  } else expecting ("keyword RETURN");

  return Node_new (N_WRITE_INST, NULL, NULL, expr);
}

/* voidInstruction -> ';'
 */
Node * rule_voidInstruction () {
  if (yycc == ';') next_token (); else expecting ("';'");
  return Node_new (N_VOID_INST, NULL, NULL, NULL);
}

/* expression -> conjunction OR expression
 * expression -> conjunction
 */
Node * rule_expression () {
  Node *conj = NULL, *expr = NULL;

  if (yycc == '('  || yycc == NUMBER || yycc == VAR_ID ||
      yycc == READ || yycc == FUN_ID) {
    conj = rule_conjunction ();

    if (yycc == OR) {
      next_token ();

      if (yycc == '('  || yycc == NUMBER || yycc == VAR_ID ||
          yycc == READ || yycc == FUN_ID) {
        expr = rule_expression ();
        Node_lastSibling (conj)->next = expr;
      } else expecting ("expression after OR");
    }

  } else expecting ("conjunction");

  if (expr != NULL) {
    return Node_new (N_OP_EXP, Value_int (OR), NULL, conj);
  }
  return conj;
}

/* conjunction -> comparison AND conjunction
 * conjunction -> comparison
 */
Node * rule_conjunction () {
  Node *comp = NULL, *conj = NULL;

  if (yycc == '(' || yycc == NUMBER || yycc == VAR_ID ||
      yycc == READ || yycc == FUN_ID) {
    comp = rule_comparison ();

    if (yycc == AND) {
      next_token ();

      if (yycc == '(' || yycc == NUMBER || yycc == VAR_ID ||
          yycc == READ || yycc == FUN_ID) {
        conj = rule_conjunction ();
        Node_lastSibling (comp)->next = conj;
      } else expecting ("expression after AND");
    }
  } else expecting ("comparison");

  if (conj != NULL) {
    return Node_new (N_OP_EXP, Value_int (AND), NULL, comp);
  }
  return comp;
}

/* comparison -> arithmeticExpr EQ arithmeticExpr
 * comparison -> arithmeticExpr NEQ arithmeticExpr
 * comparison -> arithmeticExpr '<' arithmeticExpr
 * comparison -> arithmeticExpr LE arithmeticExpr
 * comparison -> arithmeticExpr
 */
Node * rule_comparison () {
  Node  *expr1 = NULL, *expr2 = NULL;
  Value *op = NULL;

  if (yycc == '('  || yycc == NUMBER || yycc == VAR_ID ||
      yycc == READ || yycc == FUN_ID) {
    expr1= rule_arithmeticExpr ();

    if (yycc == EQ || yycc == NEQ || yycc == '<' || yycc == LE) {
      op = Value_int ((int) yycc);
      next_token ();

      if (yycc == '('  || yycc == NUMBER || yycc == VAR_ID ||
          yycc == READ || yycc == FUN_ID) {
        expr2 = rule_arithmeticExpr ();
        Node_lastSibling (expr1)->next = expr2;
      } else expecting ("arithmetic expression");
    }

  } else expecting ("arithmetic expression");

  if (expr2 != NULL) {
    return Node_new (N_OP_EXP, op, NULL, expr1);
  }
  return expr1;
}

/* arithmeticExpr -> term '+' arithmeticExpr
 * arithmeticExpr -> term '-' arithmeticExpr
 * arithmeticExpr -> term
 */
Node * rule_arithmeticExpr () {
  Node  *term = NULL, *expr = NULL;
  Value *op = NULL;

  if (yycc == '('  || yycc == NUMBER || yycc == VAR_ID ||
      yycc == READ || yycc == FUN_ID) {
    term = rule_term ();

    if (yycc == '+' || yycc == '-') {
      op = Value_int ((int) yycc);
      next_token ();
      expr = rule_arithmeticExpr ();
      Node_lastSibling (term)->next = expr;
    }
  } else expecting ("term");

  if (expr != NULL) {
    return Node_new (N_OP_EXP, op, NULL, term);
  }
  return term;
}

/* term -> factor '/' term
 * term -> factor '*' term
 * term -> factor
 */
Node * rule_term () {
  Node  *factor = NULL, *term = NULL;
  Value *op = NULL;

  if (yycc == '('  || yycc == NUMBER || yycc == VAR_ID ||
      yycc == READ || yycc == FUN_ID) {
    factor = rule_factor ();

    if (yycc == '/' || yycc == '*') {
      op = Value_int ((int) yycc);
      next_token ();
      term = rule_term ();
      Node_lastSibling (factor)->next = term;
    }
  } else expecting ("factor");

  if (term != NULL) {
    return Node_new (N_OP_EXP, op, NULL, factor);
  }
  return factor;
}

/* factor -> '(' expression ')'
 * factor -> NUMBER
 * factor -> funCall
 * factor -> variable
 * factor -> READ '(' ')'
 */
Node * rule_factor () {
  Node  *out = NULL;
  Value *val = NULL;

  if (yycc == NUMBER) {
    val = Value_int (atoi (yytext));
    next_token ();
    return Node_new (N_INT_EXP, val, NULL, NULL);

  } else if (yycc == VAR_ID) {
    out = rule_variable ();

  } else if (yycc == FUN_ID) {
    out = rule_funCall ();

  } else if (yycc == '(') {
    next_token ();
    out = rule_expression ();

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
    return Node_new (N_READ_EXP, NULL, NULL, NULL);

  } else expecting ("factor");

  return out;
}

/* variable -> VAR_ID '[' expression ']'
 * variable -> VAR_ID
 */
Node * rule_variable () {
  Node  *index = NULL;
  Value *name  = NULL;

  if (yycc == VAR_ID) {
    name = Value_str (yytext);
    next_token ();

    if (yycc == '[') {
      next_token ();
      index = rule_expression ();

      if (yycc == ']') {
        next_token ();
      } else expecting ("']'");
    }

  } else expecting ("variable identifier");

  return Node_new (N_VAR, name, index, NULL);
}

/* funCall -> FUN_ID arguments
 */
Node * rule_funCall () {
  Node  *args = NULL;
  Value *name = NULL;
  if (yycc == FUN_ID) {
    name = Value_str (yytext);
    next_token ();
    args = rule_arguments ();

  } else expecting ("function identifier");

  return Node_new (N_CALL, name, NULL, args);
}

/* arguments -> '(' exprList ')'
 */
Node * rule_arguments () {
  Node *list = NULL;

  if (yycc == '(') {
    next_token ();
    list = rule_exprList ();

    if (yycc == ')') {
      next_token ();

    } else expecting ("')'");
  } else expecting ("'('");

  return list;
}

/* exprList -> expression exprList2
 * exprList -> void
 */
Node * rule_exprList () {
  Node *head = NULL, *tail = NULL;

  if (yycc == '('  || yycc == NUMBER || yycc == VAR_ID ||
      yycc == READ || yycc == FUN_ID) {
    head = rule_expression ();

    if (yycc == ',') {
      tail = rule_exprList2 ();
      Node_lastSibling(head)->next = tail;

    }
    return head;
  }
  return NULL;
}

/* exprList2 -> ',' expression exprList2
 * exprList2 -> void
 */
Node * rule_exprList2 () {
  Node *head = NULL, *tail = NULL;

  if (yycc == ',') {
    next_token ();

    if (yycc == '('  || yycc == NUMBER || yycc == VAR_ID ||
      yycc == READ || yycc == FUN_ID) {
      head = rule_expression ();

      if (yycc == '('  || yycc == NUMBER || yycc == VAR_ID ||
          yycc == READ || yycc == FUN_ID) {
        tail = rule_exprList2 ();
        Node_lastSibling(head)->next = tail;
      }
      return head;

    } else expecting ("expression");
  }

  return NULL;
}

/* Utilitaires */

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
  /* printf ("%3d -> %s\n", yycc, yytext); */
}
