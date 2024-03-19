/* test huge V amounts, to see if the user is *counting* the value, or if they
 * are storing up messages in a mailbox.
 */

#include <usloss.h>
#include <usyscall.h>
#include <phase1.h>
#include <phase2.h>
#include <phase3_usermode.h>
#include <stdio.h>
#include <assert.h>



/* will race for CPU time */
int Child1(char *);
int Child2(char *);

/* low-priority child; only runs when the others block */
int LP_Child(char *);



/* each process saves its semaphore ID here.  The semaphores are not shared
 * except at the very end; each process does N V-ops followed by N+1 P-ops;
 * therefore, the last one blocks.  The low-priority process will supply the
 * one missing V for each semaphore, but it cannot do that until both of the
 * other child processes block.
 */
int sem1, sem2;

#define N (250*1000)



int start3(char *arg)
{
    int pid;
    int status;

    sem1 = sem2 = -1;

    USLOSS_Console("start3(): started\n");

    Spawn("Child1", Child1, "Child1", USLOSS_MIN_STACK, 4, &pid);
    USLOSS_Console("start3(): spawned the Child1 process %d\n", pid);

    Spawn("Child2", Child2, "Child2", USLOSS_MIN_STACK, 4, &pid);
    USLOSS_Console("start3(): spawned the Child2 process %d\n", pid);

    Spawn("LP_Child", LP_Child, "LP_Child", USLOSS_MIN_STACK, 5, &pid);
    USLOSS_Console("start3(): spawned the low-priority process %d\n", pid);

    Wait(&pid, &status);
    USLOSS_Console("start3(): child %d returned status of %d\n", pid, status);

    Wait(&pid, &status);
    USLOSS_Console("start3(): child %d returned status of %d\n", pid, status);

    Wait(&pid, &status);
    USLOSS_Console("start3(): child %d returned status of %d\n", pid, status);

    USLOSS_Console("start3(): done\n");
    Terminate(0);
}



int Child_common_func(char *arg, int *sem, int retval);

int Child1(char *arg)
{
    return Child_common_func(arg, &sem1, 1);
}
int Child2(char *arg)
{
    return Child_common_func(arg, &sem2, 2);
}

int Child_common_func(char *arg, int *sem, int retval)
{
    int rc = SemCreate(0, sem);
    assert(rc == 0);

    USLOSS_Console("%s(): Semaphore %d created.  I will now call V on it %d times.\n", arg, *sem, N);
    for (int i=0; i<N; i++)
    {
        rc = SemV(*sem);
        assert(rc == 0);
    }

    USLOSS_Console("%s(): V operations completed.  I will now call P on the semaphore the same number of times.\n", arg);
    for (int i=0; i<N; i++)
    {
        rc = SemP(*sem);
        assert(rc == 0);
    }

    USLOSS_Console("%s(): P operations completed.  I will now call P once more; this will force the process to block, until the Low-Priority Child is able to give us one more V operation.\n", arg);

    rc = SemP(*sem);
    assert(rc == 0);

    USLOSS_Console("%s(): Last P operation has returned.  This process will terminate.\n", arg);
    return retval;
}



int LP_Child(char *arg)
{
    USLOSS_Console("%s(): The low-priority child is finally running.  This must not happen until both Child1,Child2 have blocked on their last P operation.\n", arg);

    SemV(sem1);
    SemV(sem2);

    return 9;
}

