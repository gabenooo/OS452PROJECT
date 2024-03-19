#include <phase2.h>
#include <phase1.h>
#include <phase2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <phase1.h>



int spawn(char *name, int (*func)(char *), char *arg, int stackSize, int priority, int *pid) {
    *pid = spork(name, func, arg, stackSize, priority);
    //printf("%d\n", pid);
    //  If the process
// returns, then the trampoline will automatically call Terminate() (still from
// user mode) on behalf of this process, passing as the argument the value returned
// from the user-main function.
// The PID of the child is returned using an out parameter pid; the return
// value is 0 if the child was successfully created, and -1 if not.

    if (*pid < 0) {
        return -1; 
    }
    return 0;
    
}

void terminate(int status) {
    int pid = getpid();
    int returnStatus = 0;

    while (join(&returnStatus) != -2) {
        /* continue to try and join until it is negative 2 */
    }

    quit(status);
}

int wait(int *pid, int *status) {
    int returnStatus;
    int joinStatus = join(status);

    /* If it joined with a child then set the pid accordingly */
    if (joinStatus >= 0 ) {
        *pid = joinStatus;
    }
    
    return joinStatus;
}

void phase3_init(void){
    systemCallVec[SYS_TERMINATE] = terminate;
    systemCallVec[SYS_WAIT] = wait;
    systemCallVec[SYS_SPAWN] = spawn;
}

void phase3_start_service_processes(void){
    
}



