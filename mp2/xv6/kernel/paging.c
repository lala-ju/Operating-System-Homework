#include "param.h"
#include "types.h"
#include "memlayout.h"
#include "elf.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"
#include <stdbool.h>

/* NTU OS 2022 */
/* Page fault handler */
int handle_pgfault() {
  /* Find the address that caused the fault */
  /* uint64 va = r_stval(); */
  uint64 va = r_stval();
  va = PGROUNDDOWN(va);

  pte_t* pte = walk(myproc()->pagetable, va, 0);
  if(*pte & PTE_S)
  {
    madvise(va, PGSIZE, MADV_WILLNEED);
    return 0;
  }
  
  char *mem = kalloc();
  if (mem == 0) {
    return -1;
  } else {
    memset(mem, 0, PGSIZE);
    if(mappages(myproc()->pagetable, va, PGSIZE, (uint64)mem, PTE_V|PTE_U|PTE_R|PTE_W|PTE_X) != 0) {
      *pte |= PTE_V;
      kfree(mem);
      return -1;
    }
  }
  
  return 0;
  /* TODO */
  //panic("not implemented yet\n");
}
