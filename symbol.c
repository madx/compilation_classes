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
  s->context = Symbol_dup (context);
  s->address = 0;
  s->data    = 0;
  s->used    = false;

  return s;
}

Symbol * Symbol_dup (Symbol *s) {
  Symbol *out;

  if (NULL == s) return NULL;

  out = malloc (sizeof(*out));
  assert (NULL != out);

  out->name    = s->name;
  out->scope   = s->scope;
  out->type    = s->type;
  out->context = s->context;
  out->address = s->address;
  out->data    = s->data;
  out->used    = s->used;

  return out;
}

void Symbol_destroy (Symbol *s) {
  if (NULL != s->context) Symbol_destroy (s->context);
  free(s);
}

void Symbol_print (Symbol *s) {
  printf ("%s@%s scope: %d type: %d\n",
    s->name, ((s->context != NULL) ? s->context->name : ""),
    s->scope, s->type
  );
}

SymTable * SymTable_new (int size) {
  SymTable *st;

  st = malloc (sizeof (*st));
  assert (NULL != st);

  st->symbols = malloc (size * sizeof (*st->symbols));
  assert (NULL != st->symbols);

  st->size = size;
  st->base = st->top = 0;

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

    case N_ARR_DEC:
      SymTable_add (st,
        Symbol_new (n->value->as.string, scope, ST_ARR, context)
      );
      SymTable_build (st, n->child);
      SymTable_build (st, n->next);
      break;

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

void SymTable_add (SymTable *st, Symbol *s) {
  st->symbols[st->top] = s;
  st->top++;
}

void SymTable_print (SymTable *st) {
  int i;

  printf ("Table (size: %d)\n", st->size);

  for (i = 0; i < st->top; i++)
    if (st->symbols[i] != NULL) Symbol_print (st->symbols[i]);
}
