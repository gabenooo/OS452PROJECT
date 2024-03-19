/* basic terminate test */

#include <usloss.h>
#include <usyscall.h>
#include <phase1.h>
#include <phase2.h>
#include <phase3_usermode.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int Child1(char *);
int Child2(char *);

int sem1;


int start3(char *arg)
{
    int pid;
    int status;

    USLOSS_Console("start3(): started\n");

    Spawn("Child1", Child1, "Child1", USLOSS_MIN_STACK, 4, &pid);
    USLOSS_Console("start3(): spawned process %d\n", pid);

    Wait(&pid, &status);
    USLOSS_Console("start3(): child %d returned status of %d\n", pid, status);

    USLOSS_Console("start3(): done\n");
    Terminate(0);
}


int Child1(char *arg) 
{
    int pid, status;

    USLOSS_Console("Child1() starting\n");

    Spawn("Child2", Child2, "Child2", USLOSS_MIN_STACK, 5, &pid);
    USLOSS_Console("Child1(): spawned process %d\n", pid);

    Wait(&pid, &status);
    USLOSS_Console("Child1(): child %d returned status of %d\n", pid, status);

    Spawn("Child3", Child2, "Child3", USLOSS_MIN_STACK, 5, &pid);
    USLOSS_Console("Child1(): spawned process %d\n", pid);

    Wait(&pid, &status);
    USLOSS_Console("Child1(): child %d returned status of %d\n", pid, status);

    USLOSS_Console("Child1(): done\n");
    Terminate(9);
}


int Child2(char *arg) 
{
    if (strcmp(arg,"Child2") == 0)
    {
        USLOSS_Console("Child2(): starting\n");
        Terminate(9);
    }
    else if (strcmp(arg,"Child3") == 0)
    {
        USLOSS_Console("Child3(): starting\n");
        Terminate(10);
    }
    else
    {
        USLOSS_Console("wrong argument passed ... test failed\n");
        USLOSS_Halt(1);
    }
    return 0;
}

