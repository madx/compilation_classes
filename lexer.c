#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"

char yytext[128];
FILE *yyin;

int yylex () {
  int  i, type;
  char c, next;

  char *keywords[] = {
    "ENTIER", "DEBUT", "FIN", "SI", "ALORS", "SINON", "TANTQUE",
    "FAIRE", "RENVOYER", "APPEL", "NON", "ET", "OU", "ECRIRE", "LIRE"
  };

  strempty (yytext);

  while (EOF != (c = fgetc (yyin))) {
    switch (char_type (c)) {
      /* Ponctuation simple et double */
      case CT_SPUNC:
        sprintf (yytext, "%c", c);
        return c;
        break;
      case CT_DPUNC:
        switch (c) {
          case '=': return scan_comp_op ('=', EQ); break;
          case '>': return scan_comp_op ('>', GE); break;
          case '<': return scan_comp_op ('<', LE); break;
        }
        break;

      /* Identifiants de variables:
       *   On scanne jusqu'au premier caractère non alphanumérique et on
       *   renvoie la chaîne trouvée.
       */
      case CT_VARID:
        type = char_type (next = fgetc (yyin));
        if (type == CT_ALPHA || type == CT_DIGIT) {
          strccat (yytext, '$');
          strccat (yytext, next);

          while (EOF != (next = fgetc (yyin)))
            if (!scan_alpha_num (next)) break;

          return VAR_ID;
        } else {
          ungetc (next, yyin); /* TODO: un $ tout seul est une erreur de syntaxe */
        }
        break;

      /* Mots
       *   On scanne jusqu'au premier caractère non alphanumérique
       *   On teste pour voir si c'est un mot clef, si non c'est un
       *   nom de fonction
       */
      case CT_ALPHA:
        strccat (yytext, c);
        while (EOF != (next = fgetc (yyin)))
          if (!scan_alpha_num (next)) break;
        for (i = 0; i < 13; i++)
          if (!strcmp (keywords[i], yytext)) {
            return 300+i;
          }
        return FUN_ID;
        break;

      /* Nombres */
      case CT_DIGIT:
        strccat (yytext, c);
        while (EOF != (next = fgetc (yyin))) {
          if (char_type(next) == CT_DIGIT)
            strccat (yytext, next);
          else {
            ungetc (next, yyin);
            break;
          }
        }
        return NUMBER;
        break;

      /* Autres caractères */
      case CT_OTHER:
        sprintf (yytext, "%d", c);
        return CT_OTHER;
        break;
    }
  }

  return 0;
}

int char_type (char c) {
  if ((c >= 'A' && c <= 'Z') ||
      (c >= 'a' && c <= 'z') || c == '_' ) {
    return CT_ALPHA;
  } else if (c >= '0' && c < '9') {
    return CT_DIGIT;
  } else {
    switch(c) {
      case ' ': case '\n': case '\t': case '\r':
        return CT_WHITE;
        break;

      case '=': case '<': case '>':
        return CT_DPUNC;
        break;

      case '(': case ')': case '{': case '}': case ';': case '.':
      case '+': case '-': case '*': case '/': case '%': case ',':
        return CT_SPUNC;
        break;

      case '$':
        return CT_VARID;
        break;
    }
  }

  return CT_OTHER;
}

char * strccat (char *str, const char c) {
  int len = strlen (str);

  str[len]   = c;
  str[len+1] = 0;

  return str;
}

void strempty (char *str) {
  str[0] = '\0';
}

int scan_comp_op (char op, int code) {
  char next;
  if ((next = fgetc (yyin)) == '=') {
    strccat (yytext, op);
    strccat (yytext, '=');
    return code;
  }

  ungetc  (next, yyin);
  strccat (yytext, op);
  return op;
}

bool scan_alpha_num (char c) {
  int type;

  type = char_type (c);

  if (type == CT_ALPHA || type == CT_DIGIT) {
    strccat (yytext, c);
    return true;
  }

  ungetc (c, yyin);
  return false;
}
