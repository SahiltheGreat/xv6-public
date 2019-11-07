#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
// #include "proc_stat.h"
// struct proc_stat tate[100];
int main (int argc,char *argv[])
{

//  int pid;
// //  int status=0,a=3,b=4;	
//  pid = fork ();
//  if (pid == 0)
//   {	
//   //  exec(argv[1],argv);
//     for(int i=0;i<9;i++)
//     {
//       cprintf("%d\n",i);
//     }
//     exit();
//   }
//   else
//   {
//     int a , b ;
//     int status=waitx(&a,&b);
//     printf(1, "Wait Time = %d\n Run Time = %d with Status %d \n",a,b,status);   
//   }  
 
//  exit();

    int pid ;
    // int status = 0;
    pid = fork();
    if(pid==0)
    {
      randomfunc();
    }
    else
    {
      int a , b ;
      int status=waitx(&a,&b);
      printf(1, "Wait Time = %d\n Run Time = %d with Status %d \n",a,b,status); 
    }
    sleep(5);
    pid = fork();
    if(pid==0)
    {
      randomfunc();
    }
    else
    {
      int a , b ;
      int status=waitx(&a,&b);
      printf(1, "Wait Time = %d\n Run Time = %d with Status %d \n",a,b,status); 
    }
    sleep(5);
    pid = fork();
    if(pid==0)
    {
      randomfunc();
    }
    else
    {
      int a , b ;
      int status=waitx(&a,&b);
      printf(1, "Wait Time = %d\n Run Time = %d with Status %d \n",a,b,status); 
    }
    sleep(5);
    pid = fork();
    if(pid==0)
    {
      randomfunc();
    }
    else
    {
      int a , b ;
      int status=waitx(&a,&b);
      printf(1, "Wait Time = %d\n Run Time = %d with Status %d \n",a,b,status); 
    }
    sleep(5);
    pid = fork();
    if(pid==0)
    {
      randomfunc();
    }
    else
    {
      int a , b ;
      int status=waitx(&a,&b);
      printf(1, "Wait Time = %d\n Run Time = %d with Status %d \n",a,b,status); 
    }
    sleep(5);
    pid = fork();
    if(pid==0)
    {
      randomfunc();
    }
    else
    {
      int a , b ;
      int status=waitx(&a,&b);
      printf(1, "Wait Time = %d\n Run Time = %d with Status %d \n",a,b,status); 
    }
    sleep(5);
    pid = fork();
    if(pid==0)
    {
      randomfunc();
    }
    else
    {
      int a , b ;
      int status=waitx(&a,&b);
      printf(1, "Wait Time = %d\n Run Time = %d with Status %d \n",a,b,status); 
    }
    sleep(5);
    pid = fork();
    if(pid==0)
    {
      randomfunc();
    }
    else
    {
      int a , b ;
      int status=waitx(&a,&b);
      printf(1, "Wait Time = %d\n Run Time = %d with Status %d \n",a,b,status); 
    }
    sleep(5);
    

}