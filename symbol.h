#ifndef SYMBOL_H_
#define SYMBOL_H_

typedef struct symbol_t_ Symbol;
typedef struct sym_table_t_ SymTable;

struct symbol_t_ {
  char *name;
  int   address;
  int   data;
  bool  used;
  enum { SC_LOCAL, SC_GLOBAL, SC_ARG      } scope;
  enum { ST_INT,   ST_ARRAY,  ST_FUNCTION } type;
};

struct sym_table_t_ {
  Symbol **symbols;
  int base, top, size;
};

SymTable * SymTable_new     (int size);
void       SymTable_destroy (SymTable *table);
SymTable * SymTable_build   (Node *root);
void       SymTable_print (SymTable *st);

Symbol * Symbol_new (char *name, int address, int scope, int type, int data);
void     Symbol_destroy (Symbol *s);
void     Symbol_print (Symbol *s);

/* void entreeFonction(void); */
/* void sortieFonction(void); */
/* int rechercheExecutable(char *identif); */
/* int rechercheDeclarative(char *identif); */
/* void affiche_table(void); */

#endif
