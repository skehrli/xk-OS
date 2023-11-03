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
  
  /*
  int n = argc;

  //create temp vspace to use for initalization later
  struct vspace temp;
  vspaceinit(&temp);

  //read program and load it via spaceloadcode
  //load code for given progam at 'path' into vspace
  uint64_t rip; //first instruction for program
  int sz = vspaceloadcode(&temp, path, &rip); 
  if(sz == 0) {
    vspacefree(&temp);
    return -1;
  }

  //initialize stack region in user's address space
  //vspace begins at SZ_2G and grows down from here
  int res = vspaceinitstack(&temp, SZ_2G);
  if(res < 0) {
    vspacefree(&temp);
    return -1;
  }

  //address of string args on user stack
  uint64_t userStack_args[n+2];
  userStack_args[0] = 0x00; // address of pc for return - doesn't matter
  userStack_args[1 + n] = 0; //null terminator at end

  //write string args to user stack
  //create deep copy of arguments from old address space to new one via vscpacewriteova - to export data to a page table that isn't currently installed

  uint64_t va_space = SZ_2G; //vspace begins here
  for(int i=0; i< n; i++) {
    //uint64_t size = strlen(argv[i]) + 1;
    //va_space -= size;
    //    va_space = (((va_space)) & ~(7)); // round down


    uint64_t size = strlen(argv[i]) + 1 + 8; //add 1 for null termination -- add 8?
    va_space -= (size/8) *8;
 
    //write to address
    res = vspacewritetova(&temp, va_space, argv[i], size - 8); 
    if(res < 0) {
      vspacefree(&temp);
      return -1;
    }

    userStack_args[i+1] = va_space; //add arg to userStack
  }

  //go to address where we will write user stack args to user stack

  int copyLen = (n+2) * 8;
  va_space -=  copyLen;
 //    va_space = (((va_space)) & ~(7)); // round down

  //write pointers to string args in user stack
  //null term and return pc (garbage)
  if(vspacewritetova(&temp, va_space, (char*)userStack_args, copyLen) < 0) {
    vspacefree(&temp);
    return -1;
  }


  struct proc *p = myproc();
  //registers
  p->tf->rip = rip;
  p->tf->rsi = va_space + 8;
  p->tf->rdi = n;
  p->tf->rsp = va_space;

  //testing
   //  vspacedumpstack(&temp);

  //copy into current virtual space
  res = vspacecopy(&(myproc()->vspace), &temp);
  //free used space

  //  vspacedumpstack(&(myproc()->vspace));

  //once memory is space is ready, use vspaceinstall(myproc()) to engage 
  vspaceinstall(myproc());
  return 0;
  */

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