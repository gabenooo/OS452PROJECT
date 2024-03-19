/* Check spawn and it's return parameters work.  */

#include <stdio.h>
#include <assert.h>

#include <usloss.h>
#include <usyscall.h>
#include <phase1.h>
#include <phase2.h>
#include <phase3_usermode.h>


int Child1(char *arg) 
{
    USLOSS_Console("Child1(): started\n");
    Terminate(32);
}


int start3(char *arg)
{
    int pid,id;

    Spawn("Child1", Child1, NULL, USLOSS_MIN_STACK, 4, &pid);
    Wait(&pid, &id);
    assert(id == 32);

    USLOSS_Console("start3(): Done.\n");
    Terminate(0);
}

