#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
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
  FILE *alvcin = NULL;
  Node *ast = NULL;
  SymTable *st = NULL;
  Program *p = NULL;
  int exit_status = EXIT_SUCCESS;

  struct {
    enum { COMPILE, EXEC, BYTECODE, TRACE } mode;
    char *out, *in;
  } options = { COMPILE, NULL, NULL };

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
    fprintf (stderr, "unable to open %s for reading (%s)\n",
             options.in, strerror (errno));
    exit (errno);
  }
  yyin = alvcin;

  if (EXEC != options.mode && BYTECODE != options.mode && NULL != options.out) {
    stdout = freopen (options.out, "w+", stdout);
    if (NULL == stdout) {
      fprintf (stderr, "unable to open %s for writing (%s)\n",
               options.out, strerror (errno));
      exit (errno);
    }
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
    Program_dump (p, st);
    break;
  case EXEC:
    runvm (VM_new (p, SymTable_globalSize (st)), p, &exit_status);
    break;
  case BYTECODE: {
    VM *vm = VM_fromBytecode (alvcin);
    runvm (vm, (p = VM_extractProgram (vm)), &exit_status);
    } break;
  case TRACE:
    SymTable_print (st);
    Program_print (p);
    break;
  }

  if (NULL != p)   Program_destroy (p);
  if (NULL != st)  SymTable_destroy (st);
  if (NULL != ast) Node_destroy (ast);

  fclose (alvcin);

  exit (exit_status);
}

void runvm (VM *vm, Program *p, int *es) {
  if (VM_getDebugMode ()) {
    puts ("Running the following program: ");
    Program_print (p);
  }
  *es = VM_run (vm);
  VM_destroy (vm);
}

void fatal (char *msg) {
  fprintf (stderr, "fatal: %s [%s]\n", msg, progname);
  exit (EXIT_FAILURE);
}
