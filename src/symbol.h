#ifndef SYMBOL_H_
#define SYMBOL_H_

typedef struct symbol_t_ Symbol;
typedef struct sym_table_t_ SymTable;

struct symbol_t_ {
  char *name;
  int   address;
  int   data;
  bool  used;
  enum { SC_GLOBAL, SC_LOCAL, SC_ARG } scope;
  enum { ST_FUN, ST_ARR, ST_INT } type;
  Symbol *context;
};

struct sym_table_t_ {
  Symbol **symbols;
  int      size, count;
};

SymTable * SymTable_new     (int size);
void       SymTable_destroy (SymTable *table);
void       SymTable_build   (SymTable *st, Node *root);
void       SymTable_print   (SymTable *st);
void       SymTable_add     (SymTable *st, Symbol *s);
bool       SymTable_existsInContext (SymTable *st, char *name, Symbol *context);
Symbol *   SymTable_find    (SymTable *st, char *name, Symbol *context);
bool       SymTable_hasFailed(bool set);
void       SymTable_warnUnused (SymTable *st);

Symbol * Symbol_new (char *name, int scope, int type, Symbol *context);
void     Symbol_destroy (Symbol *s);
void     Symbol_print (Symbol *s);

#endif
