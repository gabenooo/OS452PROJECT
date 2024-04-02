
#include <usloss.h>
#include <phase1.h>
#include <phase2.h>
#include <phase3.h>
#include <phase4.h>

#include "phase3_usermode.h"
#include "phase4_usermode.h"

#include <stdio.h>
#include <assert.h>



/* a testcase can override this value in their start4() function.  But
 * start4() runs *AFTER* we create the timeout, so how does that work?
 * Simple!  We sleep the timeout proc for 1 second, then sleep for
 * this value minus 1.  :)
 */
int testcase_timeout = 10;



extern void startProcesses(void);   // this function implmemented by Russ' Phase 1 library

void startup(int argc, char **argv)
{
    phase1_init();
    phase2_init();
    phase3_init();
    phase4_init();
    startProcesses();
}



/* force the testcase driver to priority 1, instead of the
 * normal priority for testcase_main
 */
int start4(char*);
static int start4_trampoline(char*);

static int testcase_timeout_proc(char*);

int testcase_main()
{
    int pid_fork, pid_join;
    int status;

    spork("testcase_timeout", testcase_timeout_proc, "ignored", USLOSS_MIN_STACK, 5);

    pid_fork = spork("start4", start4_trampoline, "start4", 4*USLOSS_MIN_STACK, 3);
    pid_join = join(&status);

    if (pid_join != pid_fork)
    {
        USLOSS_Console("*** TESTCASE FAILURE *** - the join() pid doesn't match the fork() pid.  %d/%d\n", pid_fork,pid_join);
        USLOSS_Halt(1);
    }

    return status;
}

static int testcase_timeout_proc(char *ignored)
{
    /* so that we can use the Sleep() syscall, I force this function into usermode.
     * But if we're going to do that, why not just make this a child of start4()?
     * Simply because then Terminate() would wait to join() on this process.
     */
    if (USLOSS_PsrSet(USLOSS_PSR_CURRENT_INT) != USLOSS_DEV_OK)
    {
        USLOSS_Console("ERROR: Could not disable kernel mode.\n");
        USLOSS_Halt(1);
    }

    Sleep(1);
    Sleep(testcase_timeout-1);

    USLOSS_Console("TESTCASE TIMED OUT!!!\n");
    USLOSS_Halt(1);

    return 0;
}

static int start4_trampoline(char *arg)
{
    if (USLOSS_PsrSet(USLOSS_PSR_CURRENT_INT) != USLOSS_DEV_OK)
    {
        USLOSS_Console("ERROR: Could not disable kernel mode.\n");
        USLOSS_Halt(1);
    }

    int rc = start4(arg);

    Terminate(rc);
    return 0;    // Terminate() should never return
}



void finish      (int argc, char **argv) {}
void test_setup  (int argc, char **argv) {}
void test_cleanup(int argc, char **argv) {}



/* this is called by legacy code, in Russ' Phase 1 solution.  Students do not
 * need to implement this function.
 */
int phase2_check_io(void) { return 1; }

