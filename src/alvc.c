#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <libgen.h>
#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include "symbol.h"
#include "vm.h"
#include "alvc.h"

#define arg(s) (!strcmp(s, argv[i]))

extern char* yyfile;
extern FILE* yyin;

static char* progname;

int main (int argc, char* argv[]) {
  int i = 1;
  FILE *alvcout, *alvcin;
  Node *ast;
  SymTable *st;

  struct {
    enum { COMPILE, EXEC, BYTECODE, TRACE } mode;
    char *out, *in;
  } options = { COMPILE, "/dev/stdout", NULL };

  progname = basename (argv[0]);

  if (!(argc == 2 && arg("-h")) && argc < 3)
    fatal ("not enough arguments (run with -h)");

  if (arg("-h")) {
    puts (
      "alvc -- the Awesome L Virtual machine and Compiler\n"
      "  usage: alvc mode input [output]\n"
      "    mode:   may be either -c (compile), -x (execute),\n"
      "            -b (execute bytecode), -t (trace) ou -h (help)\n"
      "    input:  input file in L or in bytecode\n"
      "    output: output file, defaults to /dev/stdout (only for -c/-t)"
    );
    exit (EXIT_SUCCESS);
  }
  else if (arg("-c")) options.mode = COMPILE;
  else if (arg("-x")) options.mode = EXEC;
  else if (arg("-b")) options.mode = BYTECODE;
  else if (arg("-t")) options.mode = TRACE;
  else                fatal ("unknwon mode");

  yyfile     = argv[++i];
  options.in = yyfile;

  if (argv[++i]) options.out = argv[i];

  alvcin  = fopen (options.in,  "r");
  if (NULL == alvcout) {
    fprintf (stderr, "unable to open %s for reading\n", options.in);
    exit (EXIT_FAILURE);
  }
  yyin = alvcin;

  alvcout = fopen (options.out, "w+");
  if (NULL == alvcout) {
    fprintf (stderr, "unable to open %s for writing\n", options.out);
    exit (EXIT_FAILURE);
  }

  next_token ();
  ast = rule_program ();

  st = SymTable_new (
    Node_countType (ast, N_VAR_DEC) +
    Node_countType (ast, N_FUN_DEC) +
    Node_countType (ast, N_ARR_DEC)
  );
  SymTable_build (st, ast);
  if (SymTable_hasFailed (false)) exit(EXIT_FAILURE);

  switch (options.mode) {
  case COMPILE: {
    Program * p = AST_compile (ast, st);
    SymTable_print (st);
    Program_print (p);
    break;
    }
  case EXEC: {
    break;
    }
  case BYTECODE: {
    break;
    }
  case TRACE: {
    break;
    }
  }

  fclose (alvcout);
  fclose (alvcin);

  exit (EXIT_SUCCESS);
}

void fatal (char *msg) {
  fprintf(stderr, "fatal: %s [%s]\n", msg, progname);
  exit (EXIT_FAILURE);
}
