#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include "symbol.h"
#include "vm.h"

struct {
  OpCode opCode;
  bool  hasOperand;
  char *mnemonic;
} OpCodes[] = {
  { _NIL,     0,  "?"        }, { _PUSHC,   1,  "EMPC    " },
  { _PUSHL,   1,  "EMPL    " }, { _POPL,    1,  "DEPL    " },
  { _PUSHG,   1,  "EMPG    " }, { _POPG,    1,  "DEPG    " },
  { _PUSHT,   1,  "EMPT    " }, { _POPT,    1,  "DEPT    " },
  { _ADD,     0,  "ADD     " }, { _SUB,     0,  "SOUS    " },
  { _MUL,     0,  "MUL     " }, { _DIV,     0,  "DIV     " },
  { _MOD,     0,  "MOD     " }, { _EQ,      0,  "EGAL    " },
  { _LT,      0,  "INF     " }, { _LE,      0,  "INFEG   " },
  { _NOT,     0,  "NON     " }, { _READ,    0,  "LIRE    " },
  { _WRITE,   0,  "ECRIV   " }, { _JUMP,    1,  "SAUT    " },
  { _IFTRUE,  1,  "SIVRAI  " }, { _IFFALS,  1,  "SIFAUX  " },
  { _CALL,    1,  "APPEL   " }, { _RETURN,  0,  "RETOUR  " },
  { _IN,      0,  "ENTREE  " }, { _OUT,     0,  "SORTIE  " },
  { _STACK,   1,  "PILE    " }, { _STOP,    0,  "STOP    " }
};

struct {
  NodeType type;
  CompFunc func;
  char     *name;
} CompFuncs[] = {
  { N_PROGRAM,     &AST_cmp_program,     "AST_cmp_program    " },
  { N_FUN_DEC,     &AST_cmp_fun_dec,     "AST_cmp_fun_dec    " },
  { N_VAR_DEC,     &AST_cmp_var_dec,     "AST_cmp_var_dec    " },
  { N_ARR_DEC,     &AST_cmp_arr_dec,     "AST_cmp_arr_dec    " },
  { N_OP_EXP,      &AST_cmp_op_exp,      "AST_cmp_op_exp     " },
  { N_INT_EXP,     &AST_cmp_int_exp,     "AST_cmp_int_exp    " },
  { N_CALL_EXP,    &AST_cmp_call_exp,    "AST_cmp_call_exp   " },
  { N_READ_EXP,    &AST_cmp_read_exp,    "AST_cmp_read_exp   " },
  { N_CALL_INST,   &AST_cmp_call_inst,   "AST_cmp_call_inst  " },
  { N_SET_INST,    &AST_cmp_set_inst,    "AST_cmp_set_inst   " },
  { N_IF_INST,     &AST_cmp_if_inst,     "AST_cmp_if_inst    " },
  { N_WHILE_INST,  &AST_cmp_while_inst,  "AST_cmp_while_inst " },
  { N_RETURN_INST, &AST_cmp_return_inst, "AST_cmp_return_inst" },
  { N_WRITE_INST,  &AST_cmp_write_inst,  "AST_cmp_write_inst " },
  { N_VOID_INST,   &AST_cmp_void_inst,   "AST_cmp_void_inst  " },
  { N_BLOCK_INST,  &AST_cmp_block_inst,  "AST_cmp_block_inst " },
  { N_VAR,         &AST_cmp_var,         "AST_cmp_var        " },
  { N_CALL,        &AST_cmp_call,        "AST_cmp_call       " },
  { N_EXP_LIST,    &AST_cmp_exp_list,    "AST_cmp_exp_list   " },
  { N_INST_LIST,   &AST_cmp_inst_list,   "AST_cmp_inst_list  " },
  { N_DEC_LIST,    &AST_cmp_dec_list,    "AST_cmp_dec_list   " },
  { N_FAKE_NODE,   &AST_cmp_fake_node,   "AST_cmp_fake_node  " }
};

Program * Program_new (int size) {
  Program *p;

  p = malloc (sizeof (*p));
  assert (NULL != p);

  p->size = size;

  p->code = malloc (size * sizeof (p->code));
  assert (NULL != p->code);

  return p;
}

void Program_destroy (Program *p) {
  free (p->code);
  free (p);
}

void Program_print (Program *p) {
  int i, j;
  OpCode c;

  for (i = 0; i < p->size; i++) {
    c = p->code[i];
    for (j = 0; j < N_OPCODES; j++)
      if (OpCodes[j].opCode == c) break;
    if (c == _NIL) break;

    if (OpCodes[j].hasOperand) {
      printf("%4d  %s % 3d\n", i, OpCodes[j].mnemonic, p->code[i+1]);
      i++;
    } else {
      printf("%4d  %s\n", i, OpCodes[j].mnemonic);
    }
  }
}

CompContext * CompContext_new () {
  CompContext *cc;

  cc = malloc (sizeof (*cc));
  assert (NULL != cc);

  cc->context = NULL;
  cc->pc = cc->glob_c = cc->local_c = cc->arg_c = 0;

  return cc;
}

void CompContext_reset (CompContext *cc, Symbol *context) {
  cc->context = context;
  cc->local_c = cc->arg_c = 0;
}

void CompContext_destroy (CompContext *cc) {
  free (cc);
}

Program * AST_compile (Node *ast, SymTable *st) {
  Program *p;
  Symbol  *main_sym;
  CompContext *cc;

  main_sym = SymTable_find (st, "main", NULL);

  if (NULL == main_sym) {
    fputs ("error: missing main function, can't compile\n", stderr);
    exit (EXIT_FAILURE);
  }

  p = Program_new (1000);
  cc = CompContext_new ();

  AST_cmp_program (ast, st, p, cc, true);

  CompContext_destroy (cc);

  return p;
}

void AST_callCompFunc (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow) {
  int i;

  if (NULL == node) return;

  for (i = 0; i < N_COMPFUNCS; i++)
    if (CompFuncs[i].type == node->type) break;

  /* printf ("> %s, (%p:%s)\n", */
  /*         CompFuncs[i].name, (void *) node, Node_name(node)); */

  CompFuncs[i].func(node, st, p, cc, follow);
}

void AST_cmp_program (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow) {
  p->code[cc->pc++] = _STACK;
  p->code[cc->pc++] = 1;
  p->code[cc->pc++] = _CALL;
  p->code[cc->pc++] = _STOP;

  /* Compilation des déclarations globales */
  AST_callCompFunc (node->child, st, p, cc, true);
  /* Compilation des fonctions */
  if (NULL != node->child)
  AST_callCompFunc (node->child->next, st, p, cc, true);

  p->code[3] = SymTable_find (st, "main", NULL)->address;
}

void AST_cmp_fun_dec (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow) {
  Symbol *func, *old_context;
  int arg_c, local_c;

  func = SymTable_find (st, node->value->as.string, cc->context);
  func->address = cc->pc;

  old_context = cc->context;
  cc->context = func;

  p->code[cc->pc++] = _IN;

  /* Comptage du nombre d'args et de locales */
  if (NULL != node->child)
    arg_c = Node_countType (node->child->child, N_VAR_DEC) +
                Node_countType (node->child->child, N_ARR_DEC);
  if (NULL != node->child->next)
    local_c = Node_countType (node->child->next->child, N_VAR_DEC) +
                  Node_countType (node->child->next->child, N_ARR_DEC);

  /* Réservation de l'espace pour les locales */
  if (local_c != 0) {
    Node   *tmp = node->child->next;
    int locals_size = 0;
    Symbol *var;

    while (tmp != NULL) {
      var = SymTable_find (st, tmp->child->value->as.string, cc->context);
      locals_size += var->data;
      tmp = tmp->child->next;
    }

    p->code[cc->pc++] = _STACK;
    p->code[cc->pc++] = locals_size;
  }

  /* Arguments */
  AST_callCompFunc (node->child, st, p, cc, true);
  /* Locales */
  AST_callCompFunc (node->child->next, st, p, cc, true);

  /* Compilation du corps */
  AST_callCompFunc (node->child->next->next, st, p, cc, true);

  p->code[cc->pc++] = _OUT;
  p->code[cc->pc++] = _RETURN;

  CompContext_reset (cc, old_context);

  if (follow) AST_callCompFunc (node->next, st, p, cc, follow);
}

void AST_cmp_var_dec (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow) {
  Symbol *var;

  var = SymTable_find (st, node->value->as.string, cc->context);

  switch (var->scope) {
  case SC_GLOBAL:
    var->address = cc->glob_c;
    cc->glob_c += var->data;
    break;
  case SC_LOCAL:
    var->address = cc->local_c;
    cc->local_c++;
    break;
  case SC_ARG:
    var->address = cc->arg_c;
    cc->arg_c++;
    break;
  }

  if (follow) AST_callCompFunc (node->next, st, p, cc, follow);
}

void AST_cmp_arr_dec (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow) {
  if (cc->context != NULL) {
    fputs   ("fatal: arrays are forbidden inside functions\n", stderr);
    fprintf (stderr, "       arising from declaration of %s[] in %s\n",
             node->value->as.string, cc->context->name);
    exit (EXIT_FAILURE);
  }

  if (follow) AST_cmp_var_dec (node, st, p, cc, follow);
}

void AST_cmp_op_exp (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow) {
  AST_callCompFunc (node->child, st, p, cc, true);

  switch (node->value->as.number) {
  case EQ:  p->code[cc->pc++] = _EQ; break;
  case LE:  p->code[cc->pc++] = _LE; break;
  case '<': p->code[cc->pc++] = _LT; break;
  case '+': p->code[cc->pc++] = _ADD; break;
  case '-': p->code[cc->pc++] = _SUB; break;
  case '*': p->code[cc->pc++] = _MUL; break;
  case '/': p->code[cc->pc++] = _DIV; break;
  case '%': p->code[cc->pc++] = _MOD; break;
  case NEQ:
    p->code[cc->pc++] = _EQ;
    p->code[cc->pc++] = _NOT;
    break;
  case GE:
    p->code[cc->pc++] = _LT;
    p->code[cc->pc++] = _NOT;
    break;
  case '>':
    p->code[cc->pc++] = _LE;
    p->code[cc->pc++] = _NOT;
    break;
  }

  if (follow) AST_callCompFunc (node->next, st, p, cc, follow);
}

void AST_cmp_int_exp (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow) {
  p->code[cc->pc++] = _PUSHC;
  p->code[cc->pc++] = node->value->as.number;
  if (follow) AST_callCompFunc (node->next, st, p, cc, follow);
}

void AST_cmp_call_exp (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow) {
  AST_callCompFunc (node->child, st, p, cc, true);
  AST_callCompFunc (node->next, st, p, cc, true);
}

void AST_cmp_read_exp (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow) {
  AST_callCompFunc (node->child, st, p, cc, true);
  AST_callCompFunc (node->next, st, p, cc, true);
}

void AST_cmp_call_inst (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow) {
  AST_callCompFunc (node->child, st, p, cc, true);
  AST_callCompFunc (node->next, st, p, cc, true);
}

void AST_cmp_set_inst (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow) {
  Symbol *var;

  var = SymTable_find (st, node->child->value->as.string, cc->context);

  AST_callCompFunc (node->child->next, st, p, cc, true);

  switch (var->scope) {
  case SC_GLOBAL:
    if (NULL != node->child->child) {
      p->code[cc->pc++] = _PUSHC;
      p->code[cc->pc++] = AST_get_arr_index (var, node->child->child);
      p->code[cc->pc++] = _POPT;
      p->code[cc->pc++] = var->address;
    } else {
      p->code[cc->pc++] = _POPG;
      p->code[cc->pc++] = var->address;
    }
    break;
  case SC_LOCAL:
    p->code[cc->pc++] = _POPL;
    p->code[cc->pc++] = var->address;
    break;
  case SC_ARG:
    p->code[cc->pc++] = _POPL;
    p->code[cc->pc++] = -(cc->arg_c+2) + var->address;
    break;
  }

  if (follow) AST_callCompFunc (node->next, st, p, cc, follow);
}

void AST_cmp_if_inst (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow) {
  int endif, iffalse;

  /* Condition */
  AST_callCompFunc (node->child, st, p, cc, false);
  p->code[cc->pc++] = _IFFALS;
  p->code[iffalse = cc->pc++] = 0;

  /* Code si condition vraie */
  AST_callCompFunc (node->child->next, st, p, cc, false);
  p->code[iffalse] = cc->pc;

  /* Code sinon */
  if (NULL != node->child->next->next) {
    p->code[cc->pc++] = _JUMP;
    p->code[endif = cc->pc++] = 0;
    AST_callCompFunc (node->child->next->next, st, p, cc, false);
    p->code[endif] = cc->pc;
  }

  AST_callCompFunc (node->next, st, p, cc, true);
}

void AST_cmp_while_inst (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow) {
  int in, out;

  /* Condition */
  in = cc->pc;
  AST_callCompFunc (node->child, st, p, cc, false);
  p->code[cc->pc++] = _IFFALS;
  p->code[out = cc->pc++] = 0;

  /* Code si condition vraie */
  AST_callCompFunc (node->child->next, st, p, cc, false);
  p->code[cc->pc++] = _JUMP;
  p->code[cc->pc++] = in;
  p->code[out] = cc->pc;

  AST_callCompFunc (node->child, st, p, cc, true);
  AST_callCompFunc (node->next, st, p, cc, true);
}

void AST_cmp_return_inst (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow) {
  AST_callCompFunc (node->child, st, p, cc, true);

  p->code[cc->pc++] = _POPL;
  p->code[cc->pc++] = -(cc->arg_c+3);

  if (follow) AST_callCompFunc (node->next, st, p, cc, follow);
}

void AST_cmp_write_inst (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow) {
  p->code[cc->pc++] = _WRITE;
}

void AST_cmp_void_inst (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow) {
  AST_callCompFunc (node->child, st, p, cc, true);
  AST_callCompFunc (node->next, st, p, cc, true);
}

void AST_cmp_block_inst (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow) {
  AST_callCompFunc (node->child, st, p, cc, follow);
  AST_callCompFunc (node->next, st, p, cc, follow);
}

void AST_cmp_var (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow) {
  Symbol *var;
  int arr_index = 0;

  var = SymTable_find (st, node->value->as.string, cc->context);
  arr_index = AST_get_arr_index (var, node->child);

  switch (var->scope) {
  case SC_GLOBAL:
    if (NULL != node->child) {
      p->code[cc->pc++] = _PUSHC;
      p->code[cc->pc++] = AST_get_arr_index (var, node->child);
      p->code[cc->pc++] = _PUSHT;
      p->code[cc->pc++] = var->address;
    } else {
      p->code[cc->pc++] = _PUSHG;
      p->code[cc->pc++] = var->address + arr_index;
    }
    break;
  case SC_LOCAL:
    p->code[cc->pc++] = _PUSHL;
    p->code[cc->pc++] = var->address + arr_index;
    break;
  case SC_ARG:
    p->code[cc->pc++] = _PUSHL;
    p->code[cc->pc++] = -(cc->arg_c + 2) + var->address + arr_index;
    break;
  }

  if (follow) AST_callCompFunc (node->next, st, p, cc, follow);
}

void AST_cmp_call (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow) {
  AST_callCompFunc (node->child, st, p, cc, true);
  AST_callCompFunc (node->next, st, p, cc, true);
}

void AST_cmp_exp_list (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow) {
  AST_callCompFunc (node->child, st, p, cc, true);
  AST_callCompFunc (node->next, st, p, cc, true);
}

void AST_cmp_inst_list (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow) {
  AST_callCompFunc (node->child, st, p, cc, follow);
  if (follow) AST_callCompFunc (node->next, st, p, cc, follow);
}

void AST_cmp_dec_list (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow) {
  AST_callCompFunc (node->child, st, p, cc, true);
}

void AST_cmp_fake_node (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow) {
}

/* void AST_compile_recurse (Node *n, SymTable *st, Program *p) { */
/*   static Symbol *context = NULL; */
/*   static int pc = 5, glob_c = 0, local_c = 0, arg_c = 0; */
/*  */
/*   if (n == NULL) return; */
/*  */
/*   switch (n->type) { */
/*   case N_FUN_DEC: { */
/*     Symbol *func, *old_context; */
/*     int args; */
/*  */
/*     func = SymTable_find (st, n->value->as.string, context); */
/*     func->address = pc; */
/*  */
/*     p->code[pc++] = _IN; */
/*  */
/*     old_context = context; */
/*     context = func; */
/*  */
/*     local_c = arg_c = 0; */
/*  */
/*     p->code[pc++] = _STACK; */
/*     p->code[args = pc++] = 0; */
/*     AST_compile_recurse (n->child, st, p); */
/*     p->code[args] = local_c; */
/*  */
/*     context = old_context; */
/*  */
/*     p->code[pc++] = _OUT; */
/*     p->code[pc++] = _RETURN; */
/*  */
/*     AST_compile_recurse (n->next, st, p); */
/*  */
/*     } break; */
/*  */
/*   case N_VAR_DEC: case N_ARR_DEC: { */
/*     Symbol *var; */
/*     var = SymTable_find (st, n->value->as.string, context); */
/*  */
/*     switch (var->scope) { */
/*     case SC_GLOBAL: */
/*       var->address = glob_c++; */
/*       glob_c += var->data - 1; */
/*       break; */
/*     case SC_LOCAL: */
/*       var->address = local_c++; */
/*       Symbol_print (var); */
/*       local_c += var->data - 1; */
/*       break; */
/*     case SC_ARG: */
/*       var->address = arg_c++; */
/*       arg_c += var->data - 1; */
/*       break; */
/*     } */
/*  */
/*     AST_compile_recurse (n->next, st, p); */
/*  */
/*     } break; */
/*  */
/*   case N_VAR: { */
/*     Symbol *var; */
/*     var = SymTable_find (st, n->value->as.string, context); */
/*  */
/*     switch (var->scope) { */
/*     case SC_GLOBAL: */
/*       p->code[pc++] = _PUSHG; */
/*       p->code[pc++] = var->address; */
/*       break; */
/*     case SC_LOCAL: */
/*       break; */
/*     case SC_ARG: */
/*       break; */
/*     } */
/*  */
/*     AST_compile_recurse (n->next, st, p); */
/*     } break; */
/*  */
/*   case N_OP_EXP: */
/*     AST_compile_recurse (n->child, st, p); */
/*     AST_compile_recurse (n->child->next, st, p); */
/*     switch (n->value->as.number) { */
/*     case '+': p->code[pc++] = _ADD; break; */
/*     case '*': p->code[pc++] = _MUL; break; */
/*     case '-': p->code[pc++] = _SUB; break; */
/*     case '/': p->code[pc++] = _DIV; break; */
/*     case '%': p->code[pc++] = _MOD; break; */
/*     case '<': p->code[pc++] = _LT; break; */
/*     case LE:  p->code[pc++] = _LE; break; */
/*     case EQ:  p->code[pc++] = _EQ; break; */
/*     case '>': */
/*       p->code[pc++] = _LE; */
/*       p->code[pc++] = _NOT; */
/*       break; */
/*     case GE: */
/*       p->code[pc++] = _LT; */
/*       p->code[pc++] = _NOT; */
/*       break; */
/*     case NEQ: */
/*       p->code[pc++] = _EQ; */
/*       p->code[pc++] = _NOT; */
/*       break; */
/*     } */
/*     break; */
/*  */
/*   case N_INT_EXP: */
/*     p->code[pc++] = _PUSHC; */
/*     p->code[pc++] = n->value->as.number; */
/*     break; */
/*  */
/*   case N_READ_EXP: */
/*     p->code[pc++] = _READ; */
/*     break; */
/*  */
/*   case N_IF_INST: { */
/*     int jump_to; */
/*  */
/*     AST_compile_recurse (n->child, st, p); */
/*  */
/*     p->code[pc++] = _IFFALS; */
/*     p->code[jump_to = pc++] = 0; */
/*  */
/*     AST_compile_recurse (n->child->next, st, p); */
/*     p->code[jump_to] = pc; */
/*  */
/*     if (NULL != n->child->next->next) { */
/*       AST_compile_recurse (n->child->next->next, st, p); */
/*     } */
/*  */
/*     AST_compile_recurse (n->next, st, p); */
/*  */
/*     } break; */
/*  */
/*   case N_WHILE_INST: { */
/*     int jump_to, start; */
/*  */
/*     start = pc; */
/*     AST_compile_recurse (n->child, st, p); */
/*  */
/*     p->code[pc++] = _IFFALS; */
/*     p->code[jump_to = pc++] = 0; */
/*  */
/*     AST_compile_recurse (n->child->next, st, p); */
/*     p->code[pc++] = _JUMP; */
/*     p->code[pc++] = start; */
/*     p->code[jump_to] = pc; */
/*  */
/*     AST_compile_recurse (n->next, st, p); */
/*  */
/*     } break; */
/*  */
/*   case N_WRITE_INST: */
/*     AST_compile_recurse (n->child, st, p); */
/*     p->code[pc++] = _WRITE; */
/*     AST_compile_recurse (n->next, st, p); */
/*     break; */
/*  */
/*   case N_INST_LIST: */
/*     AST_compile_recurse (n->child, st, p); */
/*     break; */
/*  */
/*   default: */
/*     AST_compile_recurse (n->child, st, p); */
/*     AST_compile_recurse (n->next, st, p); */
/*   } */
/* } */

int AST_get_arr_index (Symbol *var, Node *idx) {
  int arr_index = 0;

  if (ST_ARR == var->type)
    arr_index = idx->value->as.number;
  if (arr_index > var->data - 1) {
    fprintf (stderr, "array index out of bounds (%s[%d])\n",
             var->name, arr_index);
    exit (EXIT_FAILURE);
  }

  return arr_index;
}
