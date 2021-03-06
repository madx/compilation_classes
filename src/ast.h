#ifndef AST_H_
#define AST_H_

typedef struct node_t_ Node;
typedef struct value_t_ {
  union {
    char *string;
    int   number;
  } as;
  enum { VT_STRING, VT_INT } type;
} Value;

typedef enum node_types_e_ {
  N_PROGRAM,   N_FUN_DEC,    N_VAR_DEC,    N_ARR_DEC,
  N_OP_EXP,    N_INT_EXP,    N_CALL_EXP,   N_READ_EXP,    N_DOWHILE_INST,
  N_SET_INST,  N_IF_INST,    N_WHILE_INST, N_RETURN_INST, N_WRITE_INST,
  N_VOID_INST, N_BLOCK_INST, N_VAR,        N_CALL,        N_EXP_LIST,
  N_INST_LIST, N_DEC_LIST,   N_FAKE_NODE
} NodeType;

struct node_t_ {
  NodeType type;
  Value   *value;
  Node    *next, *child;
};

Node * Node_new     (NodeType type, Value *value, Node *next, Node *child);
void   Node_destroy (Node *n);
Node * Node_lastSibling  (Node *n);
char * Node_name         (Node *n);
int    Node_countType    (Node *n, NodeType type);

void   AST_toDot         (Node *n);
void   AST_toDot_recurse (Node *parent, Node *n);
void   AST_toC           (Node *n);
void   AST_toC_recurse   (Node *parent, Node *n);

Value * Value_int (int number);
Value * Value_str (char *string);
void    Value_destroy (Value *v);

#ifdef __STRICT_ANSI__
char *strdup (const char *str);
#endif

#endif
