#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <libgen.h>
#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include "symbol.h"
#include "alvc.h"

#define arg(s) (!strcmp(s, argv[i]))

/* alvc -- le Awesome L Virtual machine and Compiler
 * usage: alvc <mode> <input> [output]
 *   mode: peut être soit -c (compiler), -x (exécuter),
 *         -b (exécuter du bytecode), ou -h (aide)
 *   input: fichier d'entrée
 *   output: fichier de sortie, par défaut /dev/stdout
 */

extern char* yyfile;

static char* progname;

int main (int argc, char* argv[]) {
  int i = 1;
  struct {
    enum { COMPILE, EXEC_PROGRAM, EXEC_BYTECODE } mode;
    char *out, *in;
  } options = { COMPILE, "/dev/stdout", NULL };

  progname = basename (argv[0]);

  if (!(argc == 2 && arg("-h")) && argc < 3)
    fatal ("not enough arguments (run with -h)");

  if (arg("-h")) {
    puts (
      "alvc -- the Awesome L Virtual machine and Compiler\n"
      "  usage: [1malvc[0m [4mmode[0m [4minput[0m [output]\n"
      "    mode:   may be either -c (compile), -x (execute),\n"
      "            -b (execute bytecode), ou -h (help)\n"
      "    input:  input file in L or in bytecode\n"
      "    output: output file, defaults to /dev/stdout (only for -c)"
    );
    exit (EXIT_SUCCESS);
  }
  else if (arg("-c")) options.mode = COMPILE;
  else if (arg("-x")) options.mode = EXEC_PROGRAM;
  else if (arg("-b")) options.mode = EXEC_BYTECODE;
  else                fatal ("unknwon mode");

  options.in = argv[++i];

  if (argv[++i]) options.out = argv[i];

  exit (EXIT_SUCCESS);
}

void fatal (char *msg) {
  fprintf(stderr, "fatal: %s [%s]\n", msg, progname);
  exit (EXIT_FAILURE);
}
