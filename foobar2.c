// #include "types.h"
// #include "stat.h"
// #include "user.h"
// #include "fs.h"

// int main(int argc, char *argv[])
// {
    // for(int i=0;i<10;i++)
    // {
    //     int pid = fork();
    //     if(pid==0)
    //     {
    //         set_priority(10-i);
    //         printf(1,"prio %d\n",10-i);
    //         // getinf(getpid());
    //         sleep(10);
    //         printf(1,"i = %d\n",10-i);
    //         exit();
    //     }
    // }
    // while(1);


//     int pid;
//   int k, n;
// //   int x, z;

//   if (argc != 2) {
//     printf(2, "usage: %s n\n", argv[0]);
//   }

//   n = atoi(argv[1]);

//   for ( k = 0; k < n; k++ ) {
//     pid = fork ();
//     set_priority(pid,10-k);
//     if ( pid < 0 ) {
//       printf(1, "%d failed in fork!\n", getpid());
//       exit();
//     } else if (pid == 0) {
//       // child
//       printf(1, "Child %d created\n",getpid());
//     //   for ( z = 0; z < 10000.0; z += 0.01 )
//         //  x =  x + 3.14 * 89.64;   // useless calculations to consume CPU time
//       exit();
//     }
//   }

//   for (k = 0; k < n; k++) {
//     wait();
//   }

//   exit();

    // int priority;
    // int pid;
    
    // if(argc<3)
    // {
    //     printf(2, "Error: Exiting\n");
    //     exit();
    // }
    // pid = atoi(argv[1]);
    // priority = atoi(argv[2]);
    // if(priority < 0 || priority > 100)  
    // {
    //     printf(2, "Invalid priority!\n");
    //     exit();
    // }
    // printf(1, "Process %d\n Priority %d\n", pid, priority);
    // set_priority(priority);
    // exit();
    // int pro1;
    // if((pro1 = fork())==0)
    // {
    //     int pro2;
    //     if((pro2 = fork())==0) {
    //         #ifdef PBS
    //         set_priority(80);
    //         #endif
    //         sleep(atoi(argv[1]));
    //         printf(1, "Exit 1\n");
    //         // getinf(getpid());
    //     }
    //     else {
    //         #ifdef PBS
    //         set_priority(20);
    //         #endif
    //         sleep(atoi(argv[2]));
    //         printf(1, "Exit 2\n");
    //         // getinf(getpid());
    //     }
    // }
    // else 
    // {
    //     #ifdef PBS
    //     set_priority(50);
    //     #endif
    //     sleep(atoi(argv[3]));
    //     printf(1, "Exit 3\n");
    //     // getinf(getpid());
    // }
    // exit();


#include "types.h"
#include "stat.h"
#include "user.h"

void looper(int f)
{
  // if(f<1)
  // return;
  // ff(f-1);
  // ff(f-2);
  // return;
  int x=0;
  for (volatile long long int i = 0; i < 1000000*f; i++)
  {
    x++;
  }
  return;
}
  
int main()
{
	// sleep(10);
 	int pid = fork();
 	if(pid == 0)
  	{
  		// sleep (10);
    	int ppid = fork();
    	if(ppid == 0)
    	{
      	#ifdef PBS
      	set_priority(80);
      	#endif
      	looper(100);
      	printf(1,"P1\n");
    	}
    	else
   	 	{
      	#ifdef PBS
      	set_priority(90);
      	#endif
      	looper(100);
      	printf(1,"P2\n");
      	wait();
    	}
  	}
  	else
  	{
   		#ifdef PBS
    	 set_priority(70);
    	#endif
    	looper(100);
    	printf(1,"P3\n");
    	wait();
  	}
  	exit();
}