#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

int main (int argc,char *argv[])
{

 int pid;
//  int status=0,a=3,b=4;	
 pid = fork ();
 if (pid == 0)
  {	
   exec(argv[1],argv);   
  // sleep(5); 
    // printf(1, "exec %s failed\n", argv[1]);
    // sleep(10);
    exit();
  }
  else
  {
  //  sleep(5);
  int a , b ;
  // getpinfo(tate,pid);
  // printf("%d %d\n",tate[0].pid[0],tate[0].runtime[0]);
  int status=waitx(&a,&b);
  printf(1, "Wait Time = %d\n Run Time = %d with Status %d \n",a,b,status); 

  //  exit();  
  }  
 
 exit();
}