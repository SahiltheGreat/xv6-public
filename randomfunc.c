#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
// #include "proc_stat.h"
// struct proc_stat tate[100];
int main (int argc,char *argv[])
{

    int x = getpid();
    int i;
    cprintf("PID is %d\n",x);
    for(i=0;i<=9;i++)
    {
        cprintf("%d ",i);
    }
    cprintf("\n");
    exit();
}