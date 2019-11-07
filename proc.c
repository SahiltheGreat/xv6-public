#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"
#include "proc_stat.h"
// #include "user.h"
// #include "printf.h"

struct proc* q0[48];
struct proc* q1[48];
struct proc* q2[48];
struct proc* q3[48];
struct proc* q4[48];
int c0 =-1;
int c1=-1;
int c2=-1;
int c3=-1;
int c4 = -1;
int timeslice[5] ={1,2,4,8,16};
struct proc_stat procstat_table;


struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

static struct proc *initproc;

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}

// Must be called with interrupts disabled
int
cpuid() {
  return mycpu()-cpus;
}

// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop.
struct cpu*
mycpu(void)
{
  int apicid, i;
  
  if(readeflags()&FL_IF)
    panic("mycpu called with interrupts enabled\n");
  
  apicid = lapicid();
  // APIC IDs are not guaranteed to be contiguous. Maybe we should have
  // a reverse map, or reserve a register to store &cpus[i].
  for (i = 0; i < ncpu; ++i) {
    if (cpus[i].apicid == apicid)
      return &cpus[i];
  }
  panic("unknown apicid\n");
}

// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure
struct proc*
myproc(void) {
  struct cpu *c;
  struct proc *p;
  pushcli();
  c = mycpu();
  p = c->proc;
  popcli();
  return p;
}

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;

  acquire(&ptable.lock);

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;


    #ifdef MLFQ
        //WHEN A PROCESS IS ALLOCATED, SET PRIORITY TO HIGHEST(0) AND SET TICKS AS 0 FOR ALL QUEUES.
        p->priority = 0;
				p->tickers = 0;
				c0++;
				q0[c0] = p;   //ASSIGN PROCESS FIRST TO HIGHEST PRIORITY QUEUE
				procstat_table.inuse[p->pid] = 1;
				procstat_table.priority[p->pid] = p->priority;
        for(int k =0 ;k<5;k++)
        {
          procstat_table.ticks[p->pid][k] = 0;
        }
				procstat_table.pid[p->pid] = p->pid;
    #endif

  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;

  #ifdef PBS
  p->priority = 60;
  #endif

  #ifdef MLFQ
        procstat_table.inuse[p->pid] = 1;
				p->priority = 0;
				p->tickers = 0;
				c0++;
				q0[c0] = p;
				procstat_table.priority[p->pid] = p->priority;
        for(int k =0 ;k<5;k++)
        {
          procstat_table.ticks[p->pid][k] = 0;
        }
				procstat_table.pid[p->pid] = p->pid;
  #endif
  release(&ptable.lock);

  // Allocate kernel stack.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;

  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;

  p->ctime = ticks; // TODO Might need to protect the read of ticks with a lock
  p->etime = 0;
  p->rtime = 0;
  p->iotime=0;
  return p;
}

//PAGEBREAK: 32
// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];

  p = allocproc();
  
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  // this assignment to p->state lets other cores
  // run this process. the acquire forces the above
  // writes to be visible, and the lock is also needed
  // because the assignment might not be atomic.
  acquire(&ptable.lock);

  p->state = RUNNABLE;

  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  struct proc *curproc = myproc();

  sz = curproc->sz;
  if(n > 0){
    if((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  curproc->sz = sz;
  switchuvm(curproc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;
  struct proc *curproc = myproc();

  // Allocate process.
  if((np = allocproc()) == 0){
    return -1;
  }

  // Copy process state from proc.
  if((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = curproc->sz;
  np->parent = curproc;
  *np->tf = *curproc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(curproc->ofile[i])
      np->ofile[i] = filedup(curproc->ofile[i]);
  np->cwd = idup(curproc->cwd);

  safestrcpy(np->name, curproc->name, sizeof(curproc->name));

  pid = np->pid;

  acquire(&ptable.lock);

  np->state = RUNNABLE;

  release(&ptable.lock);

  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *curproc = myproc();
  struct proc *p;
  int fd;

  if(curproc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(curproc->ofile[fd]){
      fileclose(curproc->ofile[fd]);
      curproc->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(curproc->cwd);
  end_op();
  curproc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(curproc->parent);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == curproc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  // Jump into the scheduler, never to return.
  curproc->state = ZOMBIE;
  curproc->etime = ticks;
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;
  struct proc *curproc = myproc();
  
  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for exited children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != curproc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->state = UNUSED;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || curproc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(curproc, &ptable.lock);  //DOC: wait-sleep
  }
}


//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  c->proc = 0;
  
  for(;;){
    // Enable interrupts on this processor.
    sti();

    // Loop over process table looking for process to run.
    acquire(&ptable.lock);




    //*******************************************************************************************
    //DEFAULT ROUND-ROBIN BASED SCHEDULING ALGORITHM
    #ifdef DEFAULT
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    {
      if(p->state != RUNNABLE)
        continue;
      c->proc = p;
      switchuvm(p);
      p->state = RUNNING;
      // cprintf("Process %s with pid %d running with create-time %d\n", p->name, p->pid, p->ctime);
      swtch(&(c->scheduler), p->context);
      switchkvm();
      // Process is done running for now.
      // It should have changed its p->state before coming back.
      c->proc = 0;
    }

    #else

    //**********************************************************************************************
    //FIRST COME FIRST SERVE SCHEDULING POLICY
    #ifdef FCFS

        struct proc *ThisProcess = 0;
        //int sometime=100;
        for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
        {
          
          if(p->state == RUNNABLE)
          {
            if(strncmp(p->name,"myprogram",9)==0 || strncmp(p->name,"pro2",4)==0)
            {
              cprintf("Process %s with ctime %d\n",p->name,p->ctime);
            }
            // if(p->pid > 1)
            // {
              if (ThisProcess!=0)
              {
                if(p->ctime < ThisProcess->ctime)
                {
                  ThisProcess = p;
                  // sometime = p->ctime;
                }
              }
              else
              {
                ThisProcess = p;
                // sometime = p->ctime;
              }
            // }
          }
          else
          {
            continue;
          }
          
        }  
        if (ThisProcess!=0)
        {

          while(ThisProcess->state==RUNNABLE)     //so that it doesn't pre-empt
          {
            p = ThisProcess;
            c->proc = p;
            switchuvm(p);
            p->state = RUNNING;
            swtch(&(c->scheduler), p->context);
            switchkvm();
            c->proc = 0;
          }
       }


    #else 

    //*********************************************************************************************
    //PRIORITY BASED SCHEDULING ALGORITHM
    #ifdef PBS     

      // cprintf("Singapore\n");
      
      for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
      {
          struct proc *ThuProcess;
          struct proc *p1;

          if(p->state != RUNNABLE)
              continue;
          ThuProcess = p;
          for(p1 = ptable.proc; p1 < &ptable.proc[NPROC]; p1++)
          {
            if(p1->state == RUNNABLE)
            {
              // cprintf("Process %s with pid %d has priority %d\n",p1->name,p1->pid,p1->priority);
              if(ThuProcess->priority > p1->priority)
              ThuProcess = p1;
            }
          }
          // cprintf("Lowest priority process %s has priority %d\n",ThuProcess->name,ThuProcess->priority);
          // if(ThisProcess != 0)
          //     p = ThisProcess;
            if (ThuProcess!=0)
            {
                p = ThuProcess;//the process with the highest priority
            }
                c->proc = p;
                switchuvm(p);
                p->state = RUNNING;
                swtch(&(c->scheduler), p->context);
                switchkvm();
                // Process is done running for now.
                // It should have changed its p->state before coming back.
                c->proc = 0;
              // }
      }
    

    #else 

    //**************************************************************************************************
    //MULTILEVEL FEEDBACK QUEUE BASED SCHEDULING ALGORITHM

    #ifdef MLFQ   
    //HIGHEST PRIORITY QUEUE
    if(c0!=-1)
    {

			for(int i=0;i<=c0;i++)
      {
          if(q0[i]->state != RUNNABLE)
							  continue;
					  p=q0[i];
					  c->proc = q0[i];
					  p->tickers++;
					  switchuvm(p);
					  p->state = RUNNING;
					  swtch(&(c->scheduler),c->proc->context);
					  switchkvm();
					  procstat_table.ticks[p->pid][0]=p->tickers;
					  if(p->tickers ==timeslice[0])
            {
              //MOVING PROCESS TO A LOWER PRIORITY QUEUE
						  c->proc->priority=(c->proc->priority)+1;
              c1++;
						  procstat_table.priority[c->proc->pid] = c->proc->priority;
						  q1[c1] = c->proc;

						  //REMOVING PROCESS FROM CURRENT QUEUE
						  q0[i]=0;
						  for(int j=i;j<=c0-1;j++)
              {
							  q0[j] = q0[j+1];
              }
						  q0[c0] = 0;
              c0--;
						  c->proc->tickers = 0;
					  }

					  c->proc = 0;
      }
    }

    //SECOND HIGHEST PRIORITY QUEUE
		if(c1!=-1)
    {
					for(int i=0;i<=c1;i++){
									  if(q1[i]->state != RUNNABLE)
										  continue;

								  p=q1[i];
								  c->proc = q1[i];
								  c->proc->tickers++;
								  switchuvm(p);
								  p->state = RUNNING;
								  swtch(&(c->scheduler), c->proc->context);
								  switchkvm();
								  procstat_table.ticks[p->pid][1]=p->tickers;;
								  if(p->tickers ==timeslice[1]){

									  //MOVING PROCESS TO A LOWER PRIORITY QUEUE
									  
									  c->proc->priority=(c->proc->priority)+1;
                    c2++;
									  procstat_table.priority[c->proc->pid] = c->proc->priority;
									  q2[c2] = c->proc;

                    //REMOVING PROCESS FROM CURRENT QUEUE
									  q1[i]=0;
									  for(int j=i;j<=c1-1;j++)
                    {
										  q1[j] = q1[j+1];
                    }
									  q1[c1] = 0;
                    c1--;
									  c->proc->tickers = 0;
									  
								  }
								  c->proc = 0;
								}
    }

        //THIRD HIGHEST PRIORITY QUEUE
				if(c2!=-1)
        {
									for(int i=0;i<=c2;i++){
													  if(q2[i]->state != RUNNABLE)
														  continue;

												  p=q2[i];
												  c->proc = q2[i];
												  c->proc->tickers++;
												  switchuvm(p);
												  p->state = RUNNING;
												  swtch(&(c->scheduler), c->proc->context);
												  switchkvm();
												  procstat_table.ticks[p->pid][2]=p->tickers;;
												  if(p->tickers ==timeslice[2]){													  
													  c->proc->priority=(c->proc->priority)+1;
                            c3++;
													  procstat_table.priority[c->proc->pid] = c->proc->priority;
													  q3[c3] = c->proc;

                            //REMOVING PROCESS FROM CURRENT QUEUE
													  q2[i]=0;
													  for(int j=i;j<=c2-1;j++)
														  q2[j] = q2[j+1];
													  q2[c2] =0;
                            c2--;
													  c->proc->tickers = 0;
													  
												  }
												  c->proc = 0;
												}
        }

        //FOURTH HIGHEST PRIORITY QUEUE
        if(c3!=-1)
        {
									for(int i=0;i<=c3;i++){
													  if(q3[i]->state != RUNNABLE)
														  continue;

												  p=q3[i];
												  c->proc = q3[i];
												  c->proc->tickers++;
												  switchuvm(p);
												  p->state = RUNNING;
												  swtch(&(c->scheduler), c->proc->context);
												  switchkvm();
												  procstat_table.ticks[p->pid][3]=p->tickers;;
												  if(p->tickers ==timeslice[3]){													  
													  c->proc->priority=(c->proc->priority)+1;
                            c4++;
													  procstat_table.priority[c->proc->pid] = c->proc->priority;
													  q4[c4] = c->proc;

                            //REMOVING PROCESS FROM CURRENT QUEUE
													  q3[i]=0;
													  for(int j=i;j<=c3-1;j++)
														  q3[j] = q3[j+1];
													  q3[c3] =0;
                            c3--;
													  c->proc->tickers = 0;
													  
												  }
												  c->proc = 0;
												}
        }

				//LOWEST PRIORITY QUEUE
        if(c4!=-1)
        {
									for(int i=0;i<=c4;i++){
													  if(q4[i]->state != RUNNABLE)
														  continue;

												  p=q4[i];
												  c->proc = q4[i];
												  c->proc->tickers++;
												  switchuvm(p);
												  p->state = RUNNING;
												  swtch(&(c->scheduler), c->proc->context);
												  switchkvm();
												  procstat_table.priority[c->proc->pid] = c->proc->priority;
												  procstat_table.ticks[c->proc->pid][4]=p->tickers;

												  q4[i]=0;
												  for(int j=i;j<=c4-1;j++)
													  q4[j] = q4[j+1];
												  q4[c4] = c->proc;

												  c->proc = 0;
												}
        }



    #endif
    #endif
    #endif
    #endif
    release(&ptable.lock);
    }
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
  int intena;
  struct proc *p = myproc();

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(mycpu()->ncli != 1)
    panic("sched locks");
  if(p->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = mycpu()->intena;
  swtch(&p->context, mycpu()->scheduler);
  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  myproc()->state = RUNNABLE;
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot
    // be run from main().
    first = 0;
    iinit(ROOTDEV);
    initlog(ROOTDEV);
  }

  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  struct proc *p = myproc();
  
  if(p == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }
  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;

  sched();

  // Tidy up.
  p->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
  {
    if(p->state == SLEEPING && p->chan == chan)
    {
      p->state = RUNNABLE;
    }
      #ifdef MLFQ
      p->tickers=0;
      if(p->priority == 0) {
				c0++;
				for(int i=c0;i>0;i--) {
					q0[i] = q0[i-1];
				}
				q0[0] = p;
			}
			else if(p->priority == 1) {
				c1++;
				for(int i=c1;i>0;i--) {
					q1[i] = q1[i-1];
				}
				q1[0] = p;
			}
			else if(p->priority == 2) {
				c2++;
				for(int i=c2;i>0;i--) {
					q2[i] = q2[i-1];
				}
				q2[0] = p;
			}
      else if(p->priority == 3) {
				c3++;
				for(int i=c3;i>0;i--) {
					q3[i] = q3[i-1];
				}
				q3[0] = p;
			}
			else  {
				c4++;
				for(int i=c4;i>0;i--) {
					q4[i] = q4[i-1];
				}
				q4[0] = p;
			}
      #endif
  }
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s %d\n", p->pid, state, p->name,p->ctime);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.Also display time taken.
int
waitx(int *wtime,int *rtime)
{
  struct proc *p;
  int havekids, pid;
  struct proc *curproc = myproc();
  
  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for exited children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != curproc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        /*CUSTOM*/
        *wtime= p->etime - p->ctime - p->rtime - p->iotime;
        *rtime=p->rtime;
        // p->ctime = 0;
        // p->etime = 5;
        // p->rtime = p->etime - p->ctime;
        // printf("%d %d\n",p->rtime,p->ctime);
        /*    */
        p->killed = 0;
        p->state = UNUSED;
        // p->priority = 0; 
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || curproc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(curproc, &ptable.lock);  //DOC: wait-sleep
  }
}

// int getpinfo(struct proc_stat * stru)
// {
//   // stru->pid = pid;
//   // stru->runtime = 0;
//   // stru->current_queue = 0;
//   // stru->num_run = 0; 
//    int i = 0;
//   acquire(&ptable.lock);
//   // Update pstat table information
//   struct proc p;
//   for(i = 0; i < NPROC; ++i) 
//   {
//     p = ptable.proc[i];
//     if(p.state == UNUSED)
//     {
//       stru->inuse[i] = 0;
//     }
//     else
//     {
//       stru->inuse[i] = 1;
//       break;
//     }
//   }  
//       stru->pid[i] = p.pid;
//       // stru->runtime[i] = p.rtime;

//   // }
//   release(&ptable.lock);
//   return 1;
// }

int set_priority(int prio)
{
  struct proc *p;
  // cprintf("PRIO is %d\n",prio); 
  int x=0;
  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
  {
    if(p->pid == myproc()->pid)
    {
        x = myproc()->priority;
        // cprintf("Priority of %d set to %d but prio is %d\n",myproc()->pid,myproc()->priority,prio);
        // prio = 40 + myproc()->pid;
        myproc()->priority = prio;
        // cprintf("pid %d priority %d\n",pid,prio);
        break;
    }
  }
  // cprintf("pid %d priority %d\n",p->pid,prio);
  // int x = myproc()->priority; 
  // myproc()->priority = prio;
  release(&ptable.lock);
  // cprintf("pid %d priority %d\n",myproc()->pid,prio);

  return x;
}

int getinf(struct rostat stru,int pid)
{
  struct proc *p = myproc();
  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
  {
    if(p->pid == pid)
    {
        // cprintf("Process with pid %d has priority %d\n",p->pid,p->priority);
        stru.pid = pid;
        stru.runtime = p->rtime;
        stru.num_run = 0;
        stru.current_queue = 0;
        break;
    }
  }

  release(&ptable.lock);
  return p->priority;
}

int getpinfo(struct rostat *stru,int pid)
{
  struct proc *p;
  int i=0;
  acquire(&ptable.lock);
  
      // if(p->pid==pid)
      // {
      //   stru->pid = pid;
      //   stru->runtime = p->rtime;
      //   stru->current_queue = 0;
      //   stru->num_run = 0;
      //   stru->ticks[0] = 0;
      //   stru->ticks[1] = 0;
      //   stru->ticks[2] = 0;
      //   stru->ticks[3] = 0;
      //   stru->ticks[4] = 0;
      // }

      for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
      {
          if(p->pid==pid)
          {
            stru->runtime = p->rtime;
            stru->pid = p->pid;
            stru->num_run = 0;
            stru->current_queue = 0;
            for(int s=0;s<5;s++)
                stru->ticks[s] =0;
          }
      }

      #ifdef MLFQ
      for(i=0;i<48;i++)
      {
          if(pid==procstat_table.pid[i])
          {
            // stru->inuse = procstat_table.inuse[i];
            // stru->pid = procstat_table.pid[i];
            stru->current_queue = procstat_table.priority[i];
            stru->num_run = 0;
            for(int j=0;j<5;j++)
            {
                  stru->ticks[j] =procstat_table.ticks[i][j] ;
            }
            break;
          }
      }
      #endif

      

  release(&ptable.lock);
  return i;
  // return p->priority;
}