/*
  Check that spawn and it's return parameters work. 
  Also check if start3 is in user mode.
*/

#include <usloss.h>
#include <usyscall.h>
#include <phase1.h>
#include <phase2.h>
#include <phase3_usermode.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>


int Child1(char *arg) 
{
    if(USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE)
    {
        USLOSS_Console("Child1(): not in user mode\n");
        USLOSS_Halt(1);
    }

    USLOSS_Console("Child1(): just started, and will end immediately.\n");
    Terminate(32);
}


int start3(char *arg)
{
    int pid,id;

    USLOSS_Console("start3(): started\n");
    if(USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE)
    {
        USLOSS_Console("start3 not in user mode\n");
        USLOSS_Halt(1);
    }

    Spawn("Child1", Child1, NULL, USLOSS_MIN_STACK, 4, &pid);
    USLOSS_Console("start3(): Spawn %d\n", pid);

    Wait(&pid, &id);
    assert(id == 32);

    USLOSS_Console("start3(): Done.\n");
    Terminate(0);
}

