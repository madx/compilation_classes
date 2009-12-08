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
  { _NIL,     0,  "?"        },
  { _PUSHC,   1,  "EMPC    " },
  { _PUSHL,   1,  "EMPL    " },
  { _POPL,    1,  "DEPL    " },
  { _PUSHG,   1,  "EMPG    " },
  { _POPG,    1,  "DEPG    " },
  { _PUSHT,   1,  "EMPT    " },
  { _POPT,    1,  "DEPT    " },
  { _ADD,     0,  "ADD     " },
  { _SUB,     0,  "SOUS    " },
  { _MUL,     0,  "MUL     " },
  { _DIV,     0,  "DIV     " },
  { _MOD,     0,  "MOD     " },
  { _EQ,      0,  "EGAL    " },
  { _LT,      0,  "INF     " },
  { _LE,      0,  "INFEG   " },
  { _NOT,     0,  "NON     " },
  { _READ,    0,  "LIRE    " },
  { _WRITE,   0,  "ECRIV   " },
  { _JUMP,    1,  "SAUT    " },
  { _IFTRUE,  1,  "SIVRAI  " },
  { _IFFALS,  1,  "SIFAUX  " },
  { _CALL,    1,  "APPEL   " },
  { _RETURN,  0,  "RETOUR  " },
  { _IN,      0,  "ENTREE  " },
  { _OUT,     0,  "SORTIE  " },
  { _STACK,   1,  "PILE    " },
  { _STOP,    0,  "STOP    " }
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

    if (OpCodes[j].hasOperand) {
      printf("%4d  %s % 3d\n", i, OpCodes[j].mnemonic, p->code[i+1]);
      i++;
    } else {
      printf("%4d  %s\n", i, OpCodes[j].mnemonic);
    }
  }
}

Program * AST_compile (Node *ast, SymTable *st) {
  Program *p;
  Symbol  *main_sym;

  main_sym = SymTable_find (st, "main", NULL);

  if (NULL == main_sym) {
    fputs ("error: missing main function, can't compile\n", stderr);
    exit (EXIT_FAILURE);
  }

  p = Program_new (1000);

  p->code[0] = _STACK;
  p->code[1] = 1;
  p->code[2] = _CALL;
  p->code[4] = _STOP;

  AST_compile_recurse (ast, st, p);

  p->code[3] = SymTable_find (st, "main", NULL)->address;

  return p;
}

void AST_compile_recurse (Node *n, SymTable *st, Program *p) {
  static int pc = 5;

  if (n == NULL) return;

  switch (n->type) {
  case N_FUN_DEC:
    {
    SymTable_find (st, n->value->as.string, NULL)->address = pc;
    p->code[pc++] = _IN;
    AST_compile_recurse (n->child, st, p);
    p->code[pc++] = _OUT;
    p->code[pc++] = _RETURN;
    AST_compile_recurse (n->next, st, p);
    }
    break;

  case N_OP_EXP:
    AST_compile_recurse (n->child, st, p);
    switch (n->value->as.number) {
    case '+': p->code[pc++] = _ADD; break;
    case '*': p->code[pc++] = _MUL; break;
    case '-': p->code[pc++] = _SUB; break;
    case '/': p->code[pc++] = _DIV; break;
    case '%': p->code[pc++] = _MOD; break;
    }
    break;


  case N_INT_EXP:
    p->code[pc++] = _PUSHC;
    p->code[pc++] = n->value->as.number;
    break;

  case N_READ_EXP:
    p->code[pc++] = _READ;
    AST_compile_recurse (n->next, st, p);
    break;

  case N_WRITE_INST:
    AST_compile_recurse (n->child, st, p);
    p->code[pc++] = _WRITE;
    AST_compile_recurse (n->next, st, p);
    break;

  default:
    AST_compile_recurse (n->child, st, p);
    AST_compile_recurse (n->next, st, p);
  }
}
