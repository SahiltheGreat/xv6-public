#ifndef _PSTAT_H_
#define _PSTAT_H_

#include "param.h"

struct proc_stat {
    int inuse[NPROC]; // whether this slot of the process table is in use (1 or 0)
    int pid[NPROC];   // PID of each process
    int priority[NPROC]; // current priority level of each process (0-4)
    int runtime[NPROC];
    int num_run[NPROC];
    // int currq[NPROC];
    int ticks[NPROC][5]; // number of ticks each process has accumulated at each of 4 priorities
};

struct rostat
{
    int pid; 
    int runtime; 
    int num_run; 
    int current_queue;
    int ticks[5];
};


#endif // _PSTAT_H_