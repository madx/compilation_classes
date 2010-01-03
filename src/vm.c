#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include "symbol.h"
#include "compile.h"
#include "vm.h"

int VM_getStackSize () {
  char *env;
  env = getenv ("STACKSIZE");
  if (NULL != env) return atoi(env); else return 1024;
}

bool VM_getDebugMode () {
  char *env;
  env = getenv ("DEBUG");
  if (NULL != env) return atoi(env) != 0; else return false;
}

VM * VM_new (Program *p, int glob_c) {
  VM *vm;

  vm = malloc (sizeof(*vm));
  assert (NULL != vm);

  vm->TP = VM_getStackSize ();

  vm->size = p->size + vm->TP + glob_c;

  vm->BEG = p->size;
  vm->BP  = vm->SP = vm->BEG + glob_c;
  vm->BEL = -1;
  vm->CO  = 0;

  vm->mem = malloc (vm->size * sizeof(*vm->mem));
  assert (NULL != vm->mem);

  vm->mem = memcpy (vm->mem, p->code, p->size * sizeof(*p->code));

  return vm;
}

Program * VM_extractProgram (VM *vm) {
  Program *p;

  p = Program_new (vm->BEG);
  p->code = memcpy (p->code, vm->mem, p->size * sizeof(*vm->mem));

  return p;
}

VM * VM_fromBytecode (FILE *stream) {
  int glob_c, prog_len, i;
  Program *p;
  VM *vm;

  if (fscanf (stream, "%d:%d", &glob_c, &prog_len) < 2) {
    fprintf (stderr, "Bytecode format error\n"
                     "Expected glob_count:prog_size:opcode1:...:opcodeN\n");
    exit (EXIT_FAILURE);
  }

  p = Program_new (prog_len);
  vm = VM_new (p, glob_c);
  Program_destroy (p);

  for (i = 0; i < prog_len; i++) {
    if (fscanf (stream, ":%d", &vm->mem[i]) < 1) {
      fprintf (stderr, "Bytecode format error\n"
                       "Expected glob_count:prog_size:opcode1:...:opcodeN\n");
      exit (EXIT_FAILURE);
    }
  }


  return vm;
}

void VM_printStack (VM *vm) {
  int i;
  if (vm->BP == vm->SP)
    printf ("[]\n");
  else {
    printf ("[");
    for (i = vm->BP; i < vm->SP - 1; i++)
      printf ("%d, ", vm->mem[i]);
    printf ("%d]\n", vm->mem[i]);
  }
}

void VM_destroy (VM *vm) {
  free (vm->mem);
  free (vm);
}

int VM_run (VM *vm) {
  int i, j;
  int exit_status = 0;
  bool debug = VM_getDebugMode ();
  bool ask_value   = false;
  bool print_value = false;

  if (debug) {
    printf ("Running with a stack of length %d.\n", VM_getStackSize ());
    printf ("CO\tSP\tBEL\tINSTR\tOP\tPILE\n");
    printf ("%d\t%d\t%d\t \t \t", vm->CO, vm->SP, vm->BEL);
    VM_printStack (vm);
  }
  for (;;) {
    if (debug) printf ("%d\t%d\t%d\t", vm->CO, vm->SP, vm->BEL);
    if (vm->CO >= vm->size) {
      printf ("\033[1;31m!!!\033[0m "
              "Stack is full, rerun after setting STACKSIZE\n");
    }
    switch (vm->mem[vm->CO++]) {
    case _PUSHC:
      if (debug) printf ("EMPC\t%d\t", vm->mem[vm->CO]);
      vm->mem[vm->SP++] = vm->mem[vm->CO++];
      break;
    case _PUSHL:
      if (debug) printf ("EMPL\t%d\t", vm->mem[vm->CO]);
      vm->mem[vm->SP++] = vm->mem[vm->BEL + vm->mem[vm->CO++]];
      break;
    case _PUSHG:
      if (debug) printf ("EMPG\t%d\t", vm->mem[vm->CO]);
      vm->mem[vm->SP++] = vm->mem[vm->BEG + vm->mem[vm->CO++]];
      break;
    case _PUSHT:
      if (debug) printf ("EMPT\t%d[%d]\t", vm->mem[vm->CO], vm->mem[vm->SP]);
      i = vm->mem[--vm->SP];
      vm->mem[vm->SP++] = vm->mem[vm->BEG + vm->mem[vm->CO++] + i];
      break;
    case _POPL:
      if (debug) printf ("DEPL\t%d\t", vm->mem[vm->CO]);
      vm->mem[vm->BEL + vm->mem[vm->CO++]] = vm->mem[--vm->SP];
      break;
    case _POPG:
      if (debug) printf ("DEPG\t%d\t", vm->mem[vm->CO]);
      vm->mem[vm->BEG + vm->mem[vm->CO++]] = vm->mem[--vm->SP];
      break;
    case _POPT:
      if (debug) printf ("DEPT\t%d[%d]\t", vm->mem[vm->CO], vm->mem[vm->SP-1]);
      i = vm->mem[--vm->SP];
      j = vm->mem[--vm->SP];
      vm->mem[vm->BEG + vm->mem[vm->CO++] + j] = i;
      break;
    case _ADD:
      if (debug) printf ("ADD\t \t");
      i = vm->mem[--vm->SP];
      vm->mem[vm->SP-1] += i;
      break;
    case _SUB:
      if (debug) printf ("SUB\t \t");
      i = vm->mem[--vm->SP];
      vm->mem[vm->SP-1] -= i;
      break;
    case _MUL:
      if (debug) printf ("MUL\t \t");
      i = vm->mem[--vm->SP];
      vm->mem[vm->SP-1] *= i;
      break;
    case _DIV:
      if (debug) printf ("DIV\t \t");
      i = vm->mem[--vm->SP];
      vm->mem[vm->SP-1] /= i;
      break;
    case _MOD:
      if (debug) printf ("MOD\t \t");
      i = vm->mem[--vm->SP];
      vm->mem[vm->SP-1] %= i;
      break;
    case _EQ:
      if (debug) printf ("EGAL\t \t");
      i = vm->mem[--vm->SP];
      vm->mem[vm->SP-1] = (i == vm->mem[vm->SP-1])? 1 : 0;
      break;
    case _LT:
      if (debug) printf ("INF\t \t");
      i = vm->mem[--vm->SP];
      vm->mem[vm->SP-1] = (vm->mem[vm->SP-1] < i)? 1 : 0;
      break;
    case _LE:
      if (debug) printf ("INFEG\t \t");
      i = vm->mem[--vm->SP];
      vm->mem[vm->SP-1] = (vm->mem[vm->SP-1] <= i)? 1 : 0;
      break;
    case _NOT:
      if (debug) printf ("NON\t \t");
      vm->mem[vm->SP - 1] = (vm->mem[vm->SP - 1] == 0)? 1 : 0;
      break;
    case _READ:
      if (debug) printf ("LIRE\t \t");
      ask_value = true;
      break;
    case _WRITE:
      if (debug) printf ("ECRIV\t \t");
      print_value = true;
      break;
    case _JUMP:
      if (debug) printf ("SAUT\t%d\t", vm->mem[vm->CO]);
      vm->CO = vm->mem[vm->CO];
      break;
    case _IFTRUE:
      if (debug) printf ("SIVRAI\t%d\t", vm->mem[vm->CO]);
      i = vm->mem[--vm->SP];
      if (i)
        vm->CO = vm->mem[vm->CO];
      else vm->CO++;
      break;
    case _IFFALS:
      if (debug) printf ("SIFAUX\t%d\t", vm->mem[vm->CO]);
      i = vm->mem[--vm->SP];
      if(i)
        vm->CO++;
      else vm->CO = vm->mem[vm->CO];
      break;
    case _CALL:
      if (debug) printf ("APPEL\t%d\t", vm->mem[vm->CO]);
      vm->mem[vm->SP++] = vm->CO + 1;
      vm->CO = vm->mem[vm->CO];
      break;
    case _RETURN:
      if (debug) printf ("RETOUR\t\t");
      vm->CO = vm->mem[--vm->SP];
      break;
    case _IN:
      if (debug) printf ("ENTREE\t\t");
      vm->mem[vm->SP++] = vm->BEL;
      vm->BEL = vm->SP;
      break;
    case _OUT:
      if (debug) printf ("SORTIE\t\t");
      vm->SP = vm->BEL;
      vm->BEL = vm->mem[--vm->SP];
      break;
    case _STACK:
      if (debug) printf ("PILE\t%d\t", vm->mem[vm->CO]);
      vm->SP += vm->mem[vm->CO++];
      break;
    case _STOP:
    case _NIL:
      if (debug) {
        printf ("STOP\t \t");
        VM_printStack (vm);
      }
      goto _exit_;
      break;
    }
    if (debug) VM_printStack (vm);
    if (ask_value) {
      printf ("\033[1;32m>>\033[0m ") ;
      scanf  ("%d", &vm->mem[vm->SP++]) ;
      ask_value = false;
    }
    if (print_value) {
      printf ("\033[1;36m=>\033[0m %d\n", vm->mem[vm->SP-1]) ;
      print_value = false;
    }
  }
  _exit_: exit_status = vm->mem[--vm->SP];
  printf ("\033[1;33m?>\033[0m %d\n", exit_status);

  return exit_status;
}
