#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "proc_stat.h"

int main(int argc, char *argv[]) {
  int pid;
  int k, n;
  int x, f=100;

  if (argc != 2) {
    printf(2, "usage: %s n\n", argv[0]);
  }

  n = atoi(argv[1]);

  for ( k = 0; k < n; k++ ) {
    pid = fork ();
    if ( pid < 0 ) {
      printf(1, "%d failed in fork!\n", getpid());
      exit();
    } else if (pid == 0)
    {
      // child
      // set_priority(60-k);
      printf(1, "Child %d created\n",getpid());
      struct rostat stru;
      getpinfo(&stru,getpid());
      sleep(10);
      for (volatile long long int i = 0; i < 1000000*f; i++)
      {
        x++;
      }
      printf(1,"pid %d runtime %d numrun %d current-queue %d\n",stru.pid,stru.runtime,stru.num_run,stru.current_queue);
      exit();
    }
  }

  for (k = 0; k < n; k++) {
    wait();
  }

  exit();
}       