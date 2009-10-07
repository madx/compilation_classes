#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ast.h"

Node * Node_new (NodeType type, void *value, Node *next, Node *child) {
  Node *n;

  n = malloc (sizeof (*n));
  assert(NULL != n);

  n->type  = type;
  n->value = value;
  n->next  = next;
  n->child = child;

  return n;
}

void Node_destroy (Node *n) {
  if (n) {
    if (n->next)  Node_destroy (n->next);
    if (n->child) Node_destroy (n->child);
    free (n);
  }
}
