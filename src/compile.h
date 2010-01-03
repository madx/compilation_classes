#ifndef COMPILE_H_
#define COMPILE_H_

/* Opcodes */
#define N_OPCODES 27
typedef enum {
  _NIL   , _PUSHC , _PUSHL, _POPL  , _PUSHG, _POPG, _PUSHT, _POPT, _ADD  , _SUB ,
  _MUL   , _DIV   , _MOD  , _EQ    , _LT   , _LE  , _NOT  , _READ, _WRITE, _JUMP,
  _IFTRUE, _IFFALS, _CALL , _RETURN, _IN   , _OUT , _STACK, _STOP
} OpCode;

typedef struct program_t_ {
  int size;
  OpCode *code;
} Program;

typedef struct comp_context_t_ {
  Symbol *context;
  int pc, glob_c, local_c, arg_c;
} CompContext;

#define N_COMPFUNCS 21
typedef void (*CompFunc)(Node *n, SymTable *s, Program *p, CompContext *cc, bool follow);

Program * Program_new     (int size);
void      Program_destroy (Program *p);
void      Program_print   (Program *p);
Program * Program_resize  (Program *p);
void      Program_dump    (Program *p, SymTable *st);

CompContext * CompContext_new ();
void          CompContext_reset (CompContext *cc, Symbol *context);
void          CompContext_destroy (CompContext *cc);

Program * AST_compile (Node *ast, SymTable *st);
int  AST_progsize     (Node *node, SymTable *st);
void AST_callCompFunc (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow);

void AST_cmp_program     (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow);
void AST_cmp_fun_dec     (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow);
void AST_cmp_var_dec     (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow);
void AST_cmp_arr_dec     (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow);
void AST_cmp_op_exp      (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow);
void AST_cmp_int_exp     (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow);
void AST_cmp_call_exp    (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow);
void AST_cmp_read_exp    (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow);
void AST_cmp_call_inst   (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow);
void AST_cmp_set_inst    (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow);
void AST_cmp_if_inst     (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow);
void AST_cmp_while_inst  (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow);
void AST_cmp_return_inst (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow);
void AST_cmp_write_inst  (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow);
void AST_cmp_void_inst   (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow);
void AST_cmp_block_inst  (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow);
void AST_cmp_var         (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow);
void AST_cmp_call        (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow);
void AST_cmp_exp_list    (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow);
void AST_cmp_inst_list   (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow);
void AST_cmp_dec_list    (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow);
void AST_cmp_fake_node   (Node *node, SymTable *st, Program *p, CompContext *cc, bool follow);

int AST_get_arr_index (Symbol *var, Node *idx);
#endif
