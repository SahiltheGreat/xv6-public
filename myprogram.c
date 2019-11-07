#include "types.h"
#include "stat.h"
#include "user.h"
 
int
main(void)
{
  // sleep(1000)
  for(long long int i=0;i<100000000;i++)
  {
    volatile int x = 69420;
    x++; 
  }
  // sleep(1000);
  printf(1, "My first xv6 program\n");
  exit();
}