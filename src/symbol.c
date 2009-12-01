#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include "symbol.h"

SymTable *yysym;

Symbol * Symbol_new (char *name, int scope, int type, Symbol *context) {
  Symbol *s;

  s = malloc (sizeof (*s));
  assert (NULL != s);

  s->name    = name;
  s->scope   = scope;
  s->type    = type;
  s->context = context;
  s->address = 0;
  s->data    = 0;
  s->used    = false;

  return s;
}

void Symbol_destroy (Symbol *s) {
  free(s);
}

void Symbol_print (Symbol *s) {
  printf ("id: %-16s context: %-16s %9p scope: %-8s type: %-6s "
          "used: %c data: %d [%p]\n",
    s->name, ((s->context != NULL) ? s->context->name : "*"),
    (void *) s->context,
    ((s->scope == SC_ARG) ? "a" : (s->scope == SC_LOCAL) ? "l" : "g"),
    ((s->type == ST_FUN) ? "func" : (s->type == ST_ARR) ? "array" : "int"),
    ((s->used) ? 'y' : 'n'),
    s->data, (void *) s
  );
}

SymTable * SymTable_new (int size) {
  SymTable *st;

  st = malloc (sizeof (*st));
  assert (NULL != st);

  st->symbols = malloc (size * sizeof (*st->symbols));
  assert (NULL != st->symbols);

  st->size  = size;
  st->count = 0;

  return st;
}

void SymTable_build (SymTable *st, Node *n) {
  static Symbol *context = NULL;
  static int scope = SC_GLOBAL;
  int old_scope = SC_GLOBAL;
  Symbol *old_context = NULL;
  Symbol *tmp;

  if (n == NULL) return;

  switch(n->type) {
    case N_VAR_DEC:
      SymTable_add (st,
        Symbol_new (n->value->as.string, scope, ST_INT, context)
      );
      SymTable_build (st, n->child);
      SymTable_build (st, n->next);
      break;

    case N_ARR_DEC: {
      Symbol *arr = Symbol_new (n->value->as.string, scope, ST_ARR, context);
      arr->data = n->child->value->as.number;
      SymTable_add (st, arr);
      SymTable_build (st, n->child);
      SymTable_build (st, n->next);
      break;
    }

    case N_FUN_DEC:
      tmp = Symbol_new (n->value->as.string, scope, ST_FUN, context);
      SymTable_add (st, tmp);

      old_context = context;
      context     = tmp;

      old_scope = scope;
      scope = SC_ARG;
      SymTable_build (st, n->child);
      scope = old_scope;

      context = old_context;

      SymTable_build (st, n->next);
      break;

    case N_DEC_LIST:
    case N_FAKE_NODE:
      if (context != NULL && context->type == ST_FUN) {
        if (scope == SC_ARG) {
          SymTable_build (st, n->child);
          scope = SC_LOCAL;
          SymTable_build (st, n->next);
          scope = SC_ARG;
        } else {
          SymTable_build (st, n->child);
          SymTable_build (st, n->next);
        }
      } else {
        SymTable_build (st, n->child);
        SymTable_build (st, n->next);
      }
      break;

    case N_VAR:
    case N_CALL:
      if (!SymTable_exists (st, n->value->as.string, context) &&
          !SymTable_exists (st, n->value->as.string, NULL)) {
        fprintf (stderr, "error: %s '%s' undeclared\n", Node_name (n),
            n->value->as.string);
        SymTable_hasFailed (true);
      }
      SymTable_build (st, n->child);
      SymTable_build (st, n->next);
      break;

    default:
      SymTable_build (st, n->child);
      SymTable_build (st, n->next);
      break;
  }
}

void SymTable_destroy (SymTable *st) {
  int i;

  for (i = 0; i < st->size; i++)
    if (st->symbols[i] != NULL) Symbol_destroy (st->symbols[i]);

  free (st->symbols);
  free (st);
}

bool SymTable_hasFailed(bool set) {
  static bool fail = false;
  if (set) fail = true;
  return fail;
}

void SymTable_add (SymTable *st, Symbol *s) {
  if (SymTable_exists (st, s->name, s->context)) {
    fprintf (stderr, "error: '%s' is already declared in %s\n",
        s->name, (NULL != s->context) ? s->context->name : "*");
    SymTable_hasFailed (true);
  }

  if (st->count != st->size) {
    st->symbols[st->count] = s;
    st->count++;
  } else {
    fprintf (stderr, "fatal: symbol table is full\n");
    exit (EXIT_FAILURE);
  }
}

bool SymTable_exists (SymTable *st, char *name, Symbol *context) {
  int i;

  for (i = 0; i < st->count; i++) {
    if (st->symbols[i]->context == context &&
        !strcmp(st->symbols[i]->name, name))
      return true;
  }

  return false;
}

void SymTable_print (SymTable *st) {
  int i;

  printf ("Table (size: %d)\n", st->size);

  for (i = 0; i < st->size; i++)
    if (st->symbols[i] != NULL) Symbol_print (st->symbols[i]);
 }
