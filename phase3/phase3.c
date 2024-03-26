#include <phase3.h>
#include <phase2.h>
#include <phase1.h>
#include <phase2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <phase1.h>
/*
 * File: phase3.c
 * Authors: Kyle Elison, Gabe Noriega
 * Class: CSC 452 Russ Lewis
 *
 * Purpose: the purpose of this code is to implement mechanisms to support
 *  user mode processes, specifically system calls.
 */


/*
 * Struct Semaphore
 * ---------------
 * this is a struct that represents a semaphore, has ID, mboxId, and semaphore value. To track these we put them in the semaphoreTable
 * 
 *  feilds:
 *  Id - the unique ID
 *  mboxId - the id of the mailbox the semaphore uses
 *  value - the value of the semaphore
 */
struct Semaphore {
    int ID;
    int mboxId;
    int value;
};
/*
 * array semaphoreTable
 * ---------------
 * this array holds the Semaphores
 */
struct Semaphore semaphoreTable[MAXSEMS];

/*
 * Function:  trampoline
 * --------------------
 * this function is the function to haldle when the proccess main gets called
 * we want the function to run in user mode, so we have this wrapper function that 
 * sets the proccess in user mode, and then calls the actual main funciton.
 * we utilize a mailbox for this funtion
 * 
 * arguments:
 * int mboxID - the mailbox id where the function pointer and arguments are stored
 * 
 */
int trampoline(int mboxId) {
    //USLOSS_Console("Trampoline called with mailbox id of %d\n", mboxId);
    /* Recieves the function pointer and arguments from the given mailbox */
    int (*func)(void*) = NULL;
    void* args = NULL;
    //USLOSS_Console("Recieving function ptr\n");
    MboxRecv(mboxId, &func, sizeof(func));
    //USLOSS_Console("Recieving args\n");
    MboxRecv(mboxId, &args, sizeof(args));
    //USLOSS_Console("Done\n");
    

    /* Then sets psr to be in user mode to run the user main */
    unsigned int psr = USLOSS_PsrGet();
    USLOSS_PsrSet( psr & ~USLOSS_PSR_CURRENT_MODE );

    int returnCode = func(args);

    Terminate(returnCode);
    USLOSS_PsrSet( psr );

    return returnCode;
}

int spawn(void* arg) {
    USLOSS_Sysargs *args = (USLOSS_Sysargs*) arg;
    
    //new mailboxs 
    
    int mbox_id = MboxCreate(2, 50);
    //USLOSS_Console("sending on id %d\n", mbox_id);
    int result1 = MboxSend(mbox_id, &args->arg1, sizeof(args->arg1));
    int result2 = MboxSend(mbox_id, &args->arg2, sizeof(args->arg2));
    
    // send trampolie with mailbox id
    args->arg1 = spork(args->arg5, trampoline, mbox_id, (int)(long)args->arg3, (int)(long)args->arg4);
    
    
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
    
    int returnStatus = 0;

    while (join(&returnStatus) != -2) {
        /* continue to try and join until it is negative 2 */
    }
    quit((long)args->arg1);
}

void wait(void* arg) {
    USLOSS_Sysargs *args = (USLOSS_Sysargs*) arg; 

    int returnStatus;
    
    //USLOSS_Console("joining\n");

    int joinStatus = join(&returnStatus);

    /* If it joined with a child then set the pid accordingly */
    if (joinStatus >= 0 ) {
        args->arg1 = joinStatus;
    }
    if (returnStatus >= 0 ) {
        args->arg2 = returnStatus;
    }
}

void semCreate(void* arg) {
    USLOSS_Sysargs *args = (USLOSS_Sysargs*) arg; 

    /* Finds the next semaphore id */
    int semID = -1;
    for (int i = 0; i < MAXSEMS; i++) {
        if (semaphoreTable[i].ID == -1) {
            semID = i;
            break;
        }
    }


    /* Instantiate the semaphore with the value */
    if (semID < 0) {
        args->arg4 = -1;
    } else {
        semaphoreTable[semID].ID = semID;
        semaphoreTable[semID].value = args->arg1;
        semaphoreTable[semID].mboxId = MboxCreate(0, 0);

        args->arg1 = semID;
        args->arg4 = 0;
    }

}

void semP(void* arg) {
    USLOSS_Sysargs *args = (USLOSS_Sysargs*) arg; 

    while (semaphoreTable[(long)args->arg1].value == 0) {
        //USLOSS_Console("Blocking pid %d\n", getpid());
        MboxRecv(semaphoreTable[(long)args->arg1].mboxId, NULL, 0);
        //USLOSS_Console("Unblocking pid %d\n", getpid());
    }

    semaphoreTable[(long)args->arg1].value--;
    //USLOSS_Console("Sem is now %d\n", semaphoreTable[(long)args->arg1].value);
}

void semV(void* arg) {
    USLOSS_Sysargs *args = (USLOSS_Sysargs*) arg; 

    semaphoreTable[(long)args->arg1].value++;
    //USLOSS_Console("Args are %d\n", (long)args->arg1);
    //USLOSS_Console("Sem is now and sending to mbox %d, %d\n", semaphoreTable[(long)args->arg1].value, semaphoreTable[(long)args->arg1].mboxId);
    int returnC = MboxCondSend(semaphoreTable[(long)args->arg1].mboxId, NULL, 0);
    //USLOSS_Console("return is %d\n", returnC);

}

void getTimeOfDay(void* arg) {
    USLOSS_Sysargs *args = (USLOSS_Sysargs*) arg; 
    args->arg1 = currentTime();
}

void getPid(void* arg) {
    USLOSS_Sysargs *args = (USLOSS_Sysargs*) arg; 
    args->arg1 = getpid();
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

    for (int i = 0; i < MAXSEMS; i++) {
        semaphoreTable[i].ID = -1;
        semaphoreTable[i].mboxId = 0;
        semaphoreTable[i].value = 0;
    }
}

void phase3_start_service_processes(void){
    
}


