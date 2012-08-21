#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <seqev.h>

#include "common.h"



int p1(void)
{
   eventcounter_t *e;

   e = create_eventcounter(EC0);

   long t = 0;

   for (int i = 0; i < 5; ++i)
   {
      eawait(e, t);

      printf("p1%ld", t);
      fflush(stdout);

      eadvance(e);
      t += 2;
   }

   return 0;
}

int p2(void)
{
   eventcounter_t *e;

   e = create_eventcounter(EC0);

   long t = 1;

   for (int i = 0; i < 5; ++i)
   {
      eawait(e, t);

      printf("p2%ld", t);
      fflush(stdout);

      eadvance(e);
      t += 2;
   }

   rm_eventcounter(e);

   return 0;
}


int main(void)
{
   /* cleanup eventcounter */
   eventcounter_t *e;
   e = create_eventcounter(EC0);
   rm_eventcounter(e);

   /* sequencer test, we don't really need it */
   sequencer_t *s;
   /* cleanup */
   s = create_sequencer(SEQ0);
   rm_sequencer(s);

   s = create_sequencer(SEQ0);
   printf("%ld", sticket(s));
   fflush(stdout);
   printf("%ldX", sticket(s));
   fflush(stdout);

   rm_sequencer(s);
   /* end sequencer test */

   pid_t p1_pid = run_child(p1);
   pid_t p2_pid = run_child(p2);

   int p1_s, p2_s;
   waitpid(p1_pid, &p1_s, 0);
   waitpid(p2_pid, &p2_s, 0);

   printf("\n");

   int ret = EXIT_FAILURE;
   if (p1_s == 0 && p2_s == 0) {
      ret = EXIT_SUCCESS;
   }
      
   return ret;
}
