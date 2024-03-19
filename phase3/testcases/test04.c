/*
 * Simple Sem Create test.
 */

#include <usloss.h>
#include <usyscall.h>
#include <phase1.h>
#include <phase2.h>
#include <phase3_usermode.h>
#include <stdio.h>

int start3(char *arg)
{
    int semaphore;
    int sem_result;

    USLOSS_Console("start3(): started.  Calling SemCreate\n");

    sem_result = SemCreate(0, &semaphore);
    USLOSS_Console("start3(): sem_result = %d, semaphore = %d\n", sem_result, semaphore);

    sem_result = SemCreate(0, &semaphore);
    USLOSS_Console("start3(): sem_result = %d, semaphore = %d\n", sem_result, semaphore);

    Terminate(0);
}

