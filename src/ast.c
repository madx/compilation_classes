#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "ast.h"
#include "lexer.h"

#ifdef __STRICT_ANSI__
char *strdup (const char *str) {
    char *dup;

    dup = malloc ( (strlen (str) + 1) * sizeof (*dup));
    assert (dup != NULL);
    strcpy (dup, str);

    return dup;
}
#endif

static char *node_names[] = {
  "program",   "fun_dec",    "var_dec",    "arr_dec",
  "op_exp",    "int_exp",    "call_exp",   "read_exp",    "call_inst",
  "set_inst",  "if_inst",    "while_inst", "return_inst", "write_inst",
  "void_inst", "block_inst", "var",        "call",        "exp_list",
  "inst_list", "dec_list",   "fake_node"
};

static char *op_str (int op) {
  switch (op) {
    case EQ:  return "=="; break;
    case NEQ: return "!="; break;
    case LE:  return "<="; break;
    case GE:  return ">="; break;
    case '<': return "<";  break;
    case '>': return ">";  break;
    case '+': return "+";  break;
    case '-': return "-";  break;
    case '*': return "×";  break;
    case '/': return "÷";  break;
    case '%': return "mod";  break;
  }
  return "unknown";
}

Node * Node_new (NodeType type, Value *value, Node *next, Node *child) {
  Node *n;

  n = malloc (sizeof (*n));
  assert (NULL != n);

  n->type  = type;
  n->value = value;
  n->next  = next;
  n->child = child;

  return n;
}

void Node_destroy (Node *n) {
  if (n != NULL) {
    if (n->next != NULL)  Node_destroy (n->next);
    if (n->child != NULL) Node_destroy (n->child);
    if (n->value != NULL) {
      if (n->value->type == VT_STRING && n->value->as.string != NULL)
        free (n->value->as.string);
      free (n->value);
    }
    free (n);
  }
}

char * Node_name (Node *n) {
  return node_names[n->type];
}

void AST_toDot (Node *n) {
  printf ("digraph {\n");
  printf ("  n%p [label=\"%s\"];\n", (void *) n, Node_name (n));
  AST_toDot_recurse (n, n->child);
  printf ("}\n");
}

void AST_toDot_recurse (Node *parent, Node *n) {
  if (n != NULL) {
    if (n->type == N_OP_EXP) {
      printf ("  n%p [label=\"%s\"];\n", (void*) n, op_str (n->value->as.number));
    } else if (n->value != NULL) {
      switch (n->value->type) {
        case VT_STRING:
          printf ("  n%p [label=\"%s (%s)\"];\n", (void*) n,
            n->value->as.string, Node_name (n)
          );
          break;
        case VT_INT:
          printf ("  n%p [label=\"%d (%s)\"];\n", (void*) n,
            n->value->as.number, Node_name (n)
          );
          break;
      }
    } else {
      printf ("  n%p [label=\"%s\"];\n", (void*) n, Node_name (n));
    }
    printf ("  n%p -> n%p;\n", (void *) parent,  (void *) n);
    if (n->next  != NULL) AST_toDot_recurse (parent, n->next);
    if (n->child != NULL) AST_toDot_recurse (n, n->child);
  }
}

Node * Node_lastSibling (Node *n) {
  Node *tmp = n;

  while (tmp->next != NULL) tmp = tmp->next;

  return tmp;
}

int Node_countType (Node *n, NodeType type) {
  int sum = 0;
  if (NULL == n) return 0;
  if (n->child != NULL) sum += Node_countType (n->child, type);
  if (n->next != NULL)  sum += Node_countType (n->next, type);

  return (n->type == type) ? 1 + sum : 0 + sum;
}

Value * Value_int (int number) {
  Value *v = NULL;

  v = malloc (sizeof (*v));
  assert (v != NULL);

  v->as.number = number;
  v->type     = VT_INT;

  return v;
}

Value * Value_str (char *string) {
  Value *v = NULL;

  v = malloc (sizeof (*v));
  assert (v != NULL);

  v->as.string = strdup (string);
  v->type     = VT_STRING;

  return v;
}
