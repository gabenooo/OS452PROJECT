/* create a bunch of semaphores, and then wakes them up in funny patterns to
 * check to see if a student has properly implmented per-semaphore wakeup
 * logic.
 *
 * In order to build a complex pattern of P/V ops, we will use a counter, and
 * which semaphore wakes up will be determined by the first prime factor of
 * that number; we will have exactly one thread per prime factor (up through
 * the 25th prime number, which is 97).
 */

#include <usloss.h>
#include <usyscall.h>
#include <phase1.h>
#include <phase2.h>
#include <phase3_usermode.h>
#include <stdio.h>
#include <assert.h>



int WorkerThread(char *);



int counter = 2;

const int PRIMES[] = { 2, 3, 5, 7,11, 13,17,19,23,29,
                      31,37,41,43,47, 53,59,61,67,71,
                      73,79,83,89,97};
#define NUM_PRIMES 25

int semaphores[NUM_PRIMES];   // parallels the PRIMES array



int start3(char *arg)
{
    int pid;
    int status;

    USLOSS_Console("start3(): started\n");

    for (int i=0; i<NUM_PRIMES; i++)
    {
        int rc = SemCreate(0, &semaphores[i]);
        assert(rc == 0);
    }

    assert(sizeof(PRIMES) / sizeof(PRIMES[0]) == NUM_PRIMES);

    for (int i=0; i<NUM_PRIMES; i++)
    {
        char *arg = (char*)(long)i;
        Spawn("Worker", WorkerThread, arg, USLOSS_MIN_STACK, 4, &pid);
        USLOSS_Console("start3(): spawned process %2d : i=%2d PRIMES[i]=%2d\n", pid, i, PRIMES[i]);
    }

    USLOSS_Console("start3(): Waking up semaphore[0], with counter=2\n");
    SemV(semaphores[0]);

    USLOSS_Console("start3(): Waiting for all of the worker processes to quit()...\n");
    for (int i=0; i<NUM_PRIMES; i++)
        Wait(&pid, &status);

    USLOSS_Console("start3(): All worker processes have quit().  Calling P() on all of the semaphores.\n");
    for (int i=0; i<NUM_PRIMES; i++)
        SemP(semaphores[i]);

    USLOSS_Console("start3(): done\n");
    Terminate(0);
}



int WorkerThread(char *arg_voidp)
{
    int arg = (int)(long)arg_voidp;
    USLOSS_Console("worker %d : started : PRIMES[arg]=%d\n", arg, PRIMES[arg]);

    while (1)
    {
        SemP(semaphores[arg]);
        USLOSS_Console("worker %d : woke up.  Counter was: %d PRIMES[%d]=%d\n", arg, counter, arg,PRIMES[arg]);
        if (counter >= PRIMES[NUM_PRIMES-1])
            break;

        counter ++;

        int found = 0;

        for (int i=0; i<NUM_PRIMES; i++)
            if (counter % PRIMES[i] == 0)
            {
                found = 1;
                USLOSS_Console("worker %d : counter=%d PRIMES[%d]=%d : calling V() semaphore[%d]\n", arg, counter, i,PRIMES[i], i);
                SemV(semaphores[i]);
                break;
            }

        assert(found);
    }

    if (counter == PRIMES[NUM_PRIMES-1])
    {
        USLOSS_Console("worker %d : I was the first to wake up after the counter hit its max value.  I will wake up all of the other processes, as well.\n", arg);

        counter++;    // so that nobody else will fall into this if() block

        for (int i=0; i<NUM_PRIMES; i++)
            if (i != arg)
                SemV(semaphores[i]);

        USLOSS_Console("worker %d : The other threads have been awoken.\n", arg);
    }

    USLOSS_Console("worker %d : Doing a V() on my own semaphore, so that start3() can P() it later.\n", arg);
    SemV(semaphores[arg]);

    USLOSS_Console("worker %d : Ending\n", arg);
    return arg+10;
}

