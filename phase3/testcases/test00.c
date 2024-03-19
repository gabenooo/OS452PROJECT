/*
 * Simple Spawn test. Child of lower priority than parent.
 */

#include <usloss.h>
#include <usyscall.h>
#include <phase1.h>
#include <phase2.h>
#include <phase3_usermode.h>
#include <stdio.h>



int Child1(char *);

int start3(char *arg)
{
    int pid;
    int status;

    USLOSS_Console("start3(): started.  Calling Spawn for Child1\n");

    Spawn("Child1", Child1, NULL, USLOSS_MIN_STACK, 5, &pid);

    USLOSS_Console("start3(): Spawn %d\n", pid);

    Wait(&pid, &status);

    USLOSS_Console("start3(): result of wait, pid = %d, status = %d\n", pid, status);
    USLOSS_Console("start3(): Parent done. Calling Terminate.\n");

    Terminate(0);
}



int Child1(char *arg) 
{
    USLOSS_Console("Child1(): starting\n");
    USLOSS_Console("Child1(): done\n");
    Terminate(9);
}

