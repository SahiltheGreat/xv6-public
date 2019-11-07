#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "proc_stat.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int 
sys_waitx(void)
{
  int *wtime;
  int *rtime;
  
  if(argptr(0, (char**)&wtime, sizeof(int)) < 0)
    return 12;

  if(argptr(1, (char**)&rtime, sizeof(int)) < 0)
    return 13;

  return waitx(wtime,rtime);
}

int sys_randomfunc(void)
{

  return 1;
}

int sys_set_priority(void)
{
  
  int prio;
  if(argint(0, &prio) < 0)
    return -1;
  // cprintf("sysproc : %d\n",prio);
  return set_priority(prio);
}

int sys_getinf(void)
{
  int pid;
	struct rostat st;
	if(argptr(0, (void*)&st, sizeof(st))<0)
				return -1;
  if(argint(1, &pid) < 0)
    return -1;
  return getinf(st,pid);
}

int sys_getpinfo(void)
{
	int pid;
	struct rostat *st;
	if(argptr(0, (void*)&st, sizeof(*st))<0)
				return -1;
  if(argint(1, &pid) < 0)
    return -1;
	// for(i=0;i<64;i++)
  // {
  //   if(pid==pstat_var.pid[i])
  //   {
  //     st->inuse[i] = pstat_var.inuse[i];
  //     st->pid[i] = pstat_var.pid[i];
  //     st->priority[i] = pstat_var.priority[i];
  //     for(j=0;j<5;j++)
  //     {
  //           st->ticks[i][j] =pstat_var.ticks[i][j] ;
  //     }
  //   }
	// }

	return getpinfo(st,pid);
}


// int sys_getpinfo(void)
// {
//   struct proc_stat * stru;
  
//   return 1;
// }