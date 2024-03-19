
#include <usloss.h>
#include <phase1.h>
#include <phase2.h>
#include <phase3.h>

#include "phase3_usermode.h"

#include <stdio.h>
#include <assert.h>



extern void startProcesses(void);   // this function implmemented by Russ' Phase 1 library

void startup(int argc, char **argv)
{
    phase1_init();
    phase2_init();
    phase3_init();
    startProcesses();
}



/* force the testcase driver to priority 1, instead of the
 * normal priority for testcase_main
 */
int start3(char*);
static int start3_trampoline(char*);

int testcase_main()
{
    int pid_fork, pid_join;
    int status;

    pid_fork = spork("start3", start3_trampoline, "start3", 4*USLOSS_MIN_STACK, 3);
    pid_join = join(&status);

    if (pid_join != pid_fork)
    {
        USLOSS_Console("*** TESTCASE FAILURE *** - the join() pid doesn't match the fork() pid.  %d/%d\n", pid_fork,pid_join);
        USLOSS_Halt(1);
    }
    else
        USLOSS_Console("TESTCASE ENDED\n");

    return status;
}

static int start3_trampoline(char *arg)
{
    if (USLOSS_PsrSet(USLOSS_PSR_CURRENT_INT) != USLOSS_DEV_OK)
    {
        USLOSS_Console("ERROR: Could not disable kernel mode.\n");
        USLOSS_Halt(1);
    }

    int rc = start3(arg);

    Terminate(rc);
    return 0;    // Terminate() should never return
}



void phase4_start_service_processes()
{
    USLOSS_Console("%s() called -- currently a NOP\n", __func__);
}

void phase5_start_service_processes()
{
    USLOSS_Console("%s() called -- currently a NOP\n", __func__);
}



void finish      (int argc, char **argv) {}
void test_setup  (int argc, char **argv) {}
void test_cleanup(int argc, char **argv) {}



/* this is called by legacy code, in Russ' Phase 1 solution.  Students do not
 * need to implement this function.
 */
int phase2_check_io(void) { return 1; }

