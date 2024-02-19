
/* Creates a 5-slot mailbox. Creates XXp1 that sends five messages to the
 * mailbox, then terminates. Creates XXp2a,b,c each of which sends a
 * message to the mailbox.  XXp2a does a conditional send, which should
 * fail.  XXp2b does a send and gets blocked.  XXp2c does a conditional
 * send which should fail.
 * Creates XXp3 which receives the six messages that should be available,
 * five from slots and one from unblocking XXp2b.
 */

#include <stdio.h>
#include <string.h>

#include <usloss.h>
#include <phase1.h>
#include <phase2.h>

int XXp1(char *);
int XXp2(char *);
int XXp3(char *);
char buf[256];

int mbox_id;



int start2(char *arg)
{
   int kid_status, kidpid;

   USLOSS_Console("start2(): started\n");

   mbox_id = MboxCreate(5, 50);
   USLOSS_Console("start2(): MboxCreate returned id = %d\n", mbox_id);

   kidpid = spork("XXp1",  XXp1, NULL,    2 * USLOSS_MIN_STACK, 1);
   kidpid = spork("XXp2a", XXp2, "XXp2a", 2 * USLOSS_MIN_STACK, 2);
   kidpid = spork("XXp2b", XXp2, "XXp2b", 2 * USLOSS_MIN_STACK, 3);
   kidpid = spork("XXp2c", XXp2, "XXp2c", 2 * USLOSS_MIN_STACK, 4);
   kidpid = spork("XXp3",  XXp3, NULL,    2 * USLOSS_MIN_STACK, 5);

   kidpid = join(&kid_status);
   USLOSS_Console("start2(): joined with kid %d, status = %d\n", kidpid, kid_status);

   kidpid = join(&kid_status);
   USLOSS_Console("start2(): joined with kid %d, status = %d\n", kidpid, kid_status);

   kidpid = join(&kid_status);
   USLOSS_Console("start2(): joined with kid %d, status = %d\n", kidpid, kid_status);

   kidpid = join(&kid_status);
   USLOSS_Console("start2(): joined with kid %d, status = %d\n", kidpid, kid_status);

   kidpid = join(&kid_status);
   USLOSS_Console("start2(): joined with kid %d, status = %d\n", kidpid, kid_status);

   quit(0);
}


int XXp1(char *arg)
{
   int i, result;
   char buffer[20];

   USLOSS_Console("XXp1(): started\n");

   for (i = 0; i < 5; i++)
   {
      USLOSS_Console("XXp1(): sending message #%d to mailbox %d\n", i, mbox_id);
      sprintf(buffer, "hello there, #%d", i);
      result = MboxSend(mbox_id, buffer, strlen(buffer)+1);
      USLOSS_Console("XXp1(): after send of message #%d, result = %d\n", i, result);
   }

   quit(3);
}


int XXp2(char *arg)
{
   int result;
   char buffer[20];

   sprintf(buffer, "hello from %s", arg);

   if ( strcmp( arg, "XXp2b" ) == 0 )
   {
      USLOSS_Console("%s(): sending message '%s' to mailbox %d, msg_size = %lu\n", arg, buffer, mbox_id, strlen(buffer)+1);
      result = MboxSend(mbox_id, buffer, strlen(buffer)+1);
      USLOSS_Console("%s(): after send of message '%s', result = %d\n", arg, buffer, result);
   }
   else
   {
      USLOSS_Console("%s(): conditionally sending message '%s' to mailbox %d, ", arg, buffer, mbox_id);
      USLOSS_Console("msg_size = %lu\n", strlen(buffer)+1);
      result = MboxCondSend(mbox_id, buffer, strlen(buffer)+1);
      USLOSS_Console("%s(): after conditional send of message '%s', result = %d\n", arg, buffer, result);
   }

   quit(4);
}


int XXp3(char *arg)
{
   char buffer[100];
   int i, result;

  /* BUGFIX: initialize buffers to predictable contents */
  memset(buffer, 'x', sizeof(buffer)-1);
  buffer[sizeof(buffer)-1] = '\0';

   USLOSS_Console("XXp3(): started\n");

   for (i = 0; i < 6; i++)
   {
      USLOSS_Console("XXp3(): receiving message #%d from mailbox %d\n", i, mbox_id);
      result = MboxRecv(mbox_id, buffer, 100);
      USLOSS_Console("XXp3(): after receipt of message #%d, result = %d   message = '%s'\n", i, result, buffer);
   }

   quit(5);
}

