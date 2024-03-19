/* recursive terminate test */

#include <usloss.h>
#include <usyscall.h>
#include <phase1.h>
#include <phase2.h>
#include <phase3_usermode.h>
#include <stdio.h>

int Child1(char *);
int Child2(char *);
int Child2a(char *);

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
    int pid;
    int status;

    GetPID(&pid);
    USLOSS_Console("%s(): starting, pid = %d\n", arg, pid);

    Spawn("Child2", Child2, "Child2", USLOSS_MIN_STACK, 2, &pid);
    USLOSS_Console("%s(): spawned process %d\n", arg, pid);

    Wait(&pid, &status);
    USLOSS_Console("%s(): child %d returned status of %d\n", arg, pid, status);

    USLOSS_Console("%s(): done\n", arg);
    Terminate(9);
}


int Child2(char *arg) 
{
    int pid;

    GetPID(&pid);
    USLOSS_Console("%s(): starting, pid = %d\n", arg, pid);

    Spawn("Child2a", Child2a, "Child2a", USLOSS_MIN_STACK, 5, &pid);
    USLOSS_Console("%s(): spawned process %d\n", arg, pid);

    USLOSS_Console("%s(): terminating\n", arg);
    Terminate(10);
}

int Child2a(char *arg) 
{
    USLOSS_Console("%s(): starting the code for Child2a\n", arg);
    Terminate(11);
}

