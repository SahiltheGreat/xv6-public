#include "types.h"
#include "stat.h"
#include "user.h"
 
int
main(int argc, char *argv[])
{
  // sleep(10);
  int x=0;
  for(volatile long long int i=0;i<1000000*atoi(argv[1]);i++)
  {
    // volatile int x = 69420;
    x++;
  }
  // sleep(10);
  printf(1, "%d\n",atoi(argv[1]));
  exit();
}