#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <libgen.h>
#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include "symbol.h"
#include "compile.h"
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
  Program *p;
  int exit_status = EXIT_SUCCESS;

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

  alvcin = fopen (options.in,  "r");
  if (NULL == alvcin) {
    fprintf (stderr, "unable to open %s for reading\n", options.in);
    exit (EXIT_FAILURE);
  }
  yyin = alvcin;

  alvcout = fopen (options.out, "w+");
  if (NULL == alvcout) {
    fprintf (stderr, "unable to open %s for writing\n", options.out);
    exit (EXIT_FAILURE);
  }

  if (options.mode != BYTECODE) {
    next_token ();
    ast = rule_program ();

    st = SymTable_new (
      Node_countType (ast, N_VAR_DEC) +
      Node_countType (ast, N_FUN_DEC) +
      Node_countType (ast, N_ARR_DEC)
    );
    SymTable_build (st, ast);
    if (SymTable_hasFailed (false)) exit(EXIT_FAILURE);

    p = AST_compile (ast, st);
  }

  switch (options.mode) {
  case COMPILE:
    Program_dump (p, st, alvcout);
    Program_destroy (p);
    break;
  case EXEC: {
    VM * vm = VM_new (p, SymTable_globalSize (st));
    int es;
    if (VM_getDebugMode ()) Program_print (p);
    printf ("\033[1;31m?>\033[0m %d\n", es = VM_run (vm));
    exit_status = es;
    VM_destroy (vm);
    Program_destroy (p);
    break;
    }
  case BYTECODE: {
    break;
    }
  case TRACE:
    SymTable_print (st);
    Program_print (p);
    Program_destroy (p);
    break;
  }

  SymTable_destroy (st);
  Node_destroy (ast);

  fclose (alvcout);
  fclose (alvcin);

  exit (exit_status);
}

void fatal (char *msg) {
  fprintf(stderr, "fatal: %s [%s]\n", msg, progname);
  exit (EXIT_FAILURE);
}
