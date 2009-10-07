#ifndef AST_H_
#define AST_H_

typedef struct node_t_ Node;

typedef enum {
  N_PROGRAM,    N_FUN_DEC,     N_VAR_DEC,     N_ARR_DEC,      N_VAR_EXP,
  N_OP_EXP,     N_INT_EXP,     N_CALL_EXP,    N_READ_EXP,     N_CALL_INST,
  N_SET_INST,   N_IF_INST,     N_WHILE_INST,  N_RETURN_INST,  N_WRITE_INST,
  N_VOID_INST,  N_BLOCK_INST,  N_VAR,         N_CALL,         N_EXP_LIST,
  N_INST_LIST,  N_DEC_LIST
} NodeType;


struct node_t_ {
  NodeType type;
  void    *value;
  Node    *next, *child;
};

Node * Node_new (NodeType type, void *value, Node *next, Node *child);
void   Node_destroy (Node *n);

#endif
