#include <phase2.h>
#include <phase1.h>
#include <phase2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <phase1.h>



int spawn(void* arg) {
    USLOSS_Sysargs *args = (USLOSS_Sysargs*) arg; 
    unsigned int psr = USLOSS_PsrGet();
    USLOSS_PsrSet( psr & ~USLOSS_PSR_CURRENT_MODE );
    args->arg1 = spork(args->arg5, args->arg1, args->arg2, (int)(long)args->arg3, (int)(long)args->arg4);
    USLOSS_PsrSet( psr );


    
    //int test = spork(name, func, arg, stackSize, priority);
    //printf("%d\n", pid);
    //  If the process
// returns, then the trampoline will automatically call Terminate() (still from
// user mode) on behalf of this process, passing as the argument the value returned
// from the user-main function.
// The PID of the child is returned using an out parameter pid; the return
// value is 0 if the child was successfully created, and -1 if not.

    if ((int)(long)args->arg1 < 0) {
        return -1; 
    }
    return 0;
    
}

void terminate(void* arg) {
    USLOSS_Sysargs *args = (USLOSS_Sysargs*) arg; 
    int pid = getpid();
    USLOSS_Console("pid to terminate is %d\n", pid);
    
    int returnStatus = 0;

    while (join(&returnStatus) != -2) {
        /* continue to try and join until it is negative 2 */
    }

    quit(args->arg1);
}

void wait(void* arg) {
    USLOSS_Sysargs *args = (USLOSS_Sysargs*) arg; 

    int returnStatus;
    
    USLOSS_Console("joining\n");

    int joinStatus = join(&returnStatus);

    USLOSS_Console("Return status is %d\n", returnStatus);

    USLOSS_Console("Join status is %d\n", joinStatus);

    /* If it joined with a child then set the pid accordingly */
    if (joinStatus >= 0 ) {
        args->arg1 = joinStatus;
    }
}

void semCreate(void* arg) {
    USLOSS_Sysargs *args = (USLOSS_Sysargs*) arg; 
}

void semP(void* arg) {
    USLOSS_Sysargs *args = (USLOSS_Sysargs*) arg; 
}

void semV(void* arg) {
    USLOSS_Sysargs *args = (USLOSS_Sysargs*) arg; 
}

void getTimeOfDay(void* arg) {
    USLOSS_Sysargs *args = (USLOSS_Sysargs*) arg; 
}

void getPid(void* arg) {
    USLOSS_Sysargs *args = (USLOSS_Sysargs*) arg; 
}


void phase3_init(void){
    
    systemCallVec[SYS_TERMINATE] = terminate;
    systemCallVec[SYS_WAIT] = wait;
    systemCallVec[SYS_SPAWN] = spawn;
    systemCallVec[SYS_SEMCREATE] = semCreate;
    systemCallVec[SYS_SEMP] = semP;
    systemCallVec[SYS_SEMV] = semV;
    systemCallVec[SYS_GETTIMEOFDAY] = getTimeOfDay;
    systemCallVec[SYS_GETPID] = getPid;
}

void phase3_start_service_processes(void){
    
}


