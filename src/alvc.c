#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include "symbol.h"
#include "alvc.h"

#define arg(s) (!strcmp(s, argv[i]))

/* tlc -- le Tiny L Compiler
 *  -c: produit un fichier .lasm
 *  -x: exécute avec tlvm
 *  -o <fichier>: écrit le code dans <fichier>
 *  FICHIER: le fichier à compiler
 */

int main (int argc, char* argv[]) {
  int i;
  struct {
    bool assemble;
    bool execute;
    char *out, *in;
  } options = { false, false, "/dev/stdout", NULL };

  for (i = 1; i < argc; i++) {
    if      (arg("-c")) options.assemble = true;
    else if (arg("-x")) options.execute  = true;
    else if (arg("-o"))
      if (NULL != argv[i+1])
        if (argv[i+1][0] == '-')
          error ("invalid argument for -o");
        else
          options.out = argv[i++];
      else error ("missing value for -o");
    else options.in = argv[i];
  }

  if (NULL == options.in)
    error ("no input file");

  exit (EXIT_SUCCESS);
}

void error (char *msg) {
  fprintf(stderr, "tlc: %s\n", msg);
  exit (EXIT_FAILURE);
}
