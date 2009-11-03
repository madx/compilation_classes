#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include "symbol.h"

SymTable *yysym;


Symbol * Symbol_new (char *name, int address, int scope, int type, int data) {
  Symbol *s;

  s = malloc (sizeof (*s));
  assert (NULL != s);

  s->name    = name;
  s->address = address;
  s->scope   = scope;
  s->type    = type;
  s->data    = data;

  return s;
}

void Symbol_destroy (Symbol *s) {
  free(s);
}

void Symbol_print (Symbol *s) {
  printf ("%s %d %d %d %d %d",
    s->name, s->address, s->data,
    s->used, s->scope, s->type
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

void SymTable_destroy (SymTable *st) {
  int i;

  for (i = 0; i < st->size; i++)
    if (st->symbols[i] != NULL) Symbol_destroy (st->symbols[i]);

  free (st->symbols);
  free (st);
}

void SymTable_print (SymTable *st) {
  int i;

  printf ("Table (size: %d)\n", st->size);

  for (i = 0; i < st->top; i++)
    if (st->symbols[i] != NULL) Symbol_print (st->symbols[i]);
}
