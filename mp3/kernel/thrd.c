#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

// for mp3
uint64
sys_thrdstop(void)
{
  int delay;
  uint64 context_id_ptr;
  uint64 handler, handler_arg;
  if (argint(0, &delay) < 0)
    return -1;
  if (argaddr(1, &context_id_ptr) < 0)
    return -1;
  if (argaddr(2, &handler) < 0)
    return -1;
  if (argaddr(3, &handler_arg) < 0)
    return -1;

  struct proc *proc = myproc();
  //TODO: mp3
  proc->delay = delay;
  proc->timercnt = 0;

  int id;
  copyin(proc->pagetable, (char*)(&id), context_id_ptr, sizeof(int));
  if(id == -1)
  {
    int suc = 0;
    for(int i = 0; i < MAX_THRD_NUM; i++)
    {
      if(proc->used[i] == 0)
      {
        proc->used[i] = 1;
        suc = 1;
        id = i;
        copyout(proc->pagetable, context_id_ptr, (char*)(&id), sizeof(int));
        break;
      }
    }
    if(suc == 0)
      return -1;
  }
  else if(id >= MAX_THRD_NUM)
    return -1;
  else if(proc->used[id] == 1)
  {
  }
  else
    return -1;
  
  proc->trap[id] = *(proc->trapframe);
  proc->temphandle = handler;
  proc->temparg = handler_arg;
  return 0;
}

// for mp3
uint64
sys_cancelthrdstop(void)
{
  int context_id, is_exit;
  if (argint(0, &context_id) < 0)
    return -1;
  if (argint(1, &is_exit) < 0)
    return -1;

  if (context_id < 0 || context_id >= MAX_THRD_NUM) {
    return -1;
  }

  struct proc *proc = myproc();

  //TODO: mp3
  if(is_exit == 0)
  {
    proc->trap[context_id] = *(proc->trapframe);
  }
  else
  {
    //release context recylcle ID
    proc->used[context_id] = 0;
  }
  int time = proc->timercnt;
  proc->timercnt = 0;
  proc->delay = 0;

  return time;
}

// for mp3
uint64
sys_thrdresume(void)
{
  int context_id;
  if (argint(0, &context_id) < 0)
    return -1;

  struct proc *proc = myproc();

  //TODO: mp3
  //out of range
  if(context_id >= MAX_THRD_NUM || context_id < 0)
    return -1;
  //not registered
  if(proc->used[context_id] == 0)
    return -1;
  //store the context back to userspace
  *(proc->trapframe) = proc->trap[context_id];
  return 0;
}
