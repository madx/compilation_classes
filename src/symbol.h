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
  int base, top, size;
};

SymTable * SymTable_new     (int size);
void       SymTable_destroy (SymTable *table);
void       SymTable_build   (SymTable *st, Node *root);
void       SymTable_print   (SymTable *st);
void       SymTable_add     (SymTable *st, Symbol *s);

Symbol * Symbol_new (char *name, int scope, int type, Symbol *context);
Symbol * Symbol_dup (Symbol *s);
void     Symbol_destroy (Symbol *s);
void     Symbol_print (Symbol *s);

/* void entreeFonction(void); */
/* void sortieFonction(void); */
/* int rechercheExecutable(char *identif); */
/* int rechercheDeclarative(char *identif); */
/* void affiche_table(void); */

#endif
