#ifndef VM_H_
#define VM_H_

typedef struct vm_t_ {
  int BEG, BEL, BP, SP, TP, CO;
  int size;
  int *mem;
} VM;

VM * VM_new          (Program *p, int glob_c);
void VM_destroy      (VM *vm);
int  VM_run          (VM *vm);
void VM_printStack   (VM *vm);
Program * VM_extractProgram (VM *vm);
VM * VM_fromBytecode (FILE *stream);

int  VM_getStackSize ();
bool VM_getDebugMode ();

#endif
