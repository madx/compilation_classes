#ifndef LEXER_H_
#define LEXER_H_

#define SCAN_BUF_LEN 256

#define INT     300
#define BEGIN   301
#define END     302
#define IF      303
#define THEN    304
#define ELSE    305
#define WHILE   306
#define DO      307
#define RETURN  308
#define CALL    309
#define NOT     310
#define AND     311
#define OR      312
#define WRITE   313
#define READ    314

#define LE      320
#define NEQ     321
#define EQ      322

#define VAR_ID  330
#define FUN_ID  331
#define NUMBER  332

typedef enum { false, true } bool;

int yylex ();

/* Permet de déterminer le type d'un opérateur de comparaison binaire */
int  scan_comp_op   (char op, int code);
bool scan_alpha_num (char c);

/* Donne le type d'un caractère (types en dessous)*/
int char_type (const char c);
#define CT_OTHER -1 /* inconnu */
#define CT_WHITE  0 /* ' ', \t, \n, \r */
#define CT_ALPHA  1 /* a-zA-Z_ */
#define CT_DIGIT  2 /* 0-9 */
#define CT_SPUNC  3 /* ( ) [ ] { } ; . , + - * / % */
#define CT_DPUNC  4 /* =[=] <[=] >[=] */
#define CT_VARID  5 /* $ */

/* Concatène un caractère dans une chaîne */
char * strccat   (char *str, const char c);

/* Met le premier caractère d'une chaîne à \0 pour la vider */
void strempty  (char *str);

#endif
