/*
 * Three process semaphore test: three processes block on semaphore, and
 * then are released with three V's.
 */

#include <usloss.h>
#include <usyscall.h>
#include <phase1.h>
#include <phase2.h>
#include <phase3_usermode.h>
#include <stdio.h>

int Child1(char *);
int Child2(char *);

int semaphore;


int start3(char *arg)
{
    int pid1,pid2,pid3, pid4;
    int sem_result;

    USLOSS_Console("start3(): started.  Creating semaphore.\n");

    sem_result = SemCreate(0, &semaphore);
    if (sem_result != 0) {
        USLOSS_Console("start3(): got non-zero semaphore result. Terminating...\n");
        Terminate(1);
    }

    USLOSS_Console("start3(): calling Spawn for Child1 (three times)\n");
    Spawn("Child1a", Child1, "Child1a", USLOSS_MIN_STACK, 2, &pid1);
    Spawn("Child1b", Child1, "Child1b", USLOSS_MIN_STACK, 2, &pid2);
    Spawn("Child1c", Child1, "Child1c", USLOSS_MIN_STACK, 2, &pid3);
    USLOSS_Console("start3(): after spawn of %d %d %d\n", pid1,pid2,pid3);

    USLOSS_Console("start3(): calling Spawn for Child2\n");
    Spawn("Child2", Child2, NULL, USLOSS_MIN_STACK, 2, &pid4);
    USLOSS_Console("start3(): after spawn of %d\n", pid4);

    USLOSS_Console("start3(): Parent done. Calling Terminate.\n");
    Terminate(0);
}


int Child1(char *arg) 
{
    USLOSS_Console("%s(): starting, P'ing semaphore\n", arg);
    SemP(semaphore);
    USLOSS_Console("%s(): done\n", arg);

    return 9;
}


int Child2(char *arg) 
{
    int pid;
    GetPID(&pid);

    USLOSS_Console("Child2(): %d starting, V'ing semaphore\n", pid );
    SemV(semaphore);
    SemV(semaphore);
    SemV(semaphore);
    USLOSS_Console("Child2(): done\n");

    return 9;
}

