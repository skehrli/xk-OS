#include <cdefs.h>
#include <defs.h>
#include <elf.h>
#include <memlayout.h>
#include <mmu.h>
#include <param.h>
#include <proc.h>
#include <trap.h>
#include <x86_64.h>

int exec(char *path, char **argv) {
  // Get the number of arguments (argv[argc] = NULL) - OK
  int argc = 0;
  
  while (argv[argc] != '\0') {
    argc++;
  }
  
  // Create a new vspace for the new process - OK
  struct vspace vs;
  if (vspaceinit(&vs) < 0) {
    return -1;
  }

  // Load the code for the new process - OK
  uint64_t entry;
  if (vspaceloadcode(&vs, path, &entry) == 0) {
    vspacefree(&vs);
    return -1;
  }

  // Initialize the stack region in the new vspace - OK
  uint64_t va_start = SZ_2G;
  if (vspaceinitstack(&vs, va_start) < 0) {
    vspacefree(&vs);
    return -1;
  }

  // Stack for user with bottom shifted by one because of PC - OK
  uint64_t user_stack[argc+2];
  uint64_t PC = 0x00;  // PC does not matter
  user_stack[0] = PC;
  user_stack[argc+1] = 0;

  // Write the arguments to the stack - OK
  uint64_t va = va_start;
  for (int arg = 0; arg < argc; arg++) {
    //int i = arg + 1;
    
    int size = strlen(argv[arg]) + 1;
    size = (size + 7) & ~7;

    va -= size;

    if (vspacewritetova(&vs, va, (char*)argv[arg], size) < 0) {
      vspacefree(&vs);
      return -1;
    }

    user_stack[arg+1] = va;
  }

  int argv_size = 8 * (argc + 2);
  va -= argv_size;

  // Write the arguments array to the stack - OK
  if (vspacewritetova(&vs, va, (char*)user_stack, argv_size) < 0) {
    vspacefree(&vs);
    return -1;
  }

  //vspacedumpstack(&vs);

  struct vspace old_vs = myproc()->vspace;

  // Set the current process's registers - OK
  struct proc *p = myproc();
  p->vspace = vs;
  p->tf->rip = entry;
  p->tf->rdi = argc;
  p->tf->rsi = va + 8;
  p->tf->rsp = va;

  vspaceinstall(myproc());

  vspacefree(&old_vs);
  return 0;
}