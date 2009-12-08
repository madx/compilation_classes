#ifndef VM_H_
#define VM_H_

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

typedef struct vm_t_ {
  int BEG, BEL, BP, SP;
  int *mem;
} VM;

Program * Program_new     (int size);
void      Program_destroy (Program *p);
void      Program_print   (Program *p);

Program * AST_compile         (Node *ast, SymTable *st);
void      AST_compile_recurse (Node *ast, SymTable *st, Program *p);

#endif
