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

/*
 * Function:  spawn
 * ----------------
 * Spawns a new process.
 * This function spawns a new process by creating a mailbox and sending
 * the necessary function arguments to the trampoline function.
 * 
 * arguments:
 *     void* arg - Pointer to the system arguments structure (USLOSS_Sysargs)
 *         arg1: function pointer
 *         arg2: argument to pass to the function
 *         arg3: the stack size
 *         arg4: priority of the new process
 *         arg5: process name
 * 
 * returns:
 *     int - 0 on success, -1 on failure
 */
int spawn(void* arg) {
    USLOSS_Sysargs *args = (USLOSS_Sysargs*) arg;
    
    /* Creates the mailbox and sends the function arguments needed so the trampoline can call it */
    int mbox_id = MboxCreate(2, 50);
    int result1 = MboxSend(mbox_id, &args->arg1, sizeof(args->arg1));
    int result2 = MboxSend(mbox_id, &args->arg2, sizeof(args->arg2));
    
    args->arg1 = spork(args->arg5, trampoline, mbox_id, (int)(long)args->arg3, (int)(long)args->arg4);

    if ((int)(long)args->arg1 < 0) {
        return -1; 
    }
    return 0;
}

/*
 * Function:  terminate
 * --------------------
 * Terminates the current process.
 * This function terminates the current process by repeatedly attempting to join
 * with child processes until the return status is -2, indicating no more children.
 * Then, it calls the `quit` function to exit the current process with the given status.
 * 
 * arguments:
 *     void* arg - Pointer to the system arguments structure (USLOSS_Sysargs)
 *         arg1: exit status for the current process
 * 
 * returns:
 *     void
 */
void terminate(void* arg) {
    USLOSS_Sysargs *args = (USLOSS_Sysargs*) arg; 
    
    /* continue to try and join until it is negative 2 */
    int returnStatus = 0;
    while (join(&returnStatus) != -2) { }
    quit((long)args->arg1);
}

/*
 * Function:  wait
 * ---------------
 * Waits for a child process to terminate.
 * This function waits for a child process to terminate and retrieves its
 * process ID and return status.
 * 
 * arguments:
 *     void* arg - Pointer to the system arguments structure (USLOSS_Sysargs)
 * 
 * returns:
 *     void
 */
void wait(void* arg) {
    USLOSS_Sysargs *args = (USLOSS_Sysargs*) arg; 

    int returnStatus;
    int joinStatus = join(&returnStatus);

    /* If it joined with a child then set the pid accordingly */
    if (joinStatus >= 0 ) {
        args->arg1 = joinStatus;
    }
    if (returnStatus >= 0 ) {
        args->arg2 = returnStatus;
    }
}

/*
 * Function:  semCreate
 * --------------------
 * Creates a new semaphore.
 * This function creates a new semaphore with the specified initial value.
 * 
 * arguments:
 *     void* arg - Pointer to the system arguments structure (USLOSS_Sysargs)
 *         arg1: initial value for the semaphore
 * 
 * returns:
 *     void
 */
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

/*
 * Function:  semP
 * ---------------
 * Semaphore P operation.
 * This function performs the "P" operation on a semaphore.
 * It decrements the value of the semaphore and blocks the process
 * if the semaphore value is 0, by waiting on the semaphore's mailbox.
 * 
 * arguments:
 *     void* arg - Pointer to the system arguments structure (USLOSS_Sysargs)
 * 
 * returns:
 *     void
 */
void semP(void* arg) {
    USLOSS_Sysargs *args = (USLOSS_Sysargs*) arg; 

    while (semaphoreTable[(long)args->arg1].value == 0) {
        MboxRecv(semaphoreTable[(long)args->arg1].mboxId, NULL, 0);
    }
    semaphoreTable[(long)args->arg1].value--;
}

/*
 * Function:  semV
 * ---------------
 * Semaphore V operation.
 * This function performs the "V" operation on a semaphore.
 * It increments the value of the semaphore and performs a conditional send
 * on the semaphore's mailbox to release a blocked process.
 * 
 * arguments:
 *     void* arg - Pointer to the system arguments structure (USLOSS_Sysargs)
 * 
 * returns:
 *     void
 */
void semV(void* arg) {
    USLOSS_Sysargs *args = (USLOSS_Sysargs*) arg; 

    semaphoreTable[(long)args->arg1].value++;
    MboxCondSend(semaphoreTable[(long)args->arg1].mboxId, NULL, 0);
}

/*
 * Function:  getTimeOfDay
 * -----------------------
 * Sets the current time of day in the system arguments.
 * This function retrieves the current time of day and stores it
 * in the system arguments structure.
 * 
 * arguments:
 *     void* arg - Pointer to the system arguments structure (USLOSS_Sysargs)
 * 
 * returns:
 *     void
 */
void getTimeOfDay(void* arg) {
    USLOSS_Sysargs *args = (USLOSS_Sysargs*) arg; 
    args->arg1 = currentTime();
}

/*
 * Function:  getPid
 * -----------------
 * Sets the process ID in the system arguments.
 * This function retrieves the process ID of the current process
 * and stores it in the system arguments structure.
 * 
 * arguments:
 *     void* arg - Pointer to the system arguments structure (USLOSS_Sysargs)
 * 
 * returns:
 *     void
 */
void getPid(void* arg) {
    USLOSS_Sysargs *args = (USLOSS_Sysargs*) arg; 
    args->arg1 = getpid();
}

/*
 * Function:  phase3_init
 * ----------------------
 * Initializes semaphore table.
 * This function iterates through the semaphore table and initializes each entry.
 * 
 * No arguments.
 * 
 * returns:
 *     void
 */
void phase3_init(void){
    for (int i = 0; i < MAXSEMS; i++) {
        semaphoreTable[i].ID = -1;
        semaphoreTable[i].mboxId = 0;
        semaphoreTable[i].value = 0;
    }
}

/*
 * Function:  phase3_start_service_processes
 * -----------------------------------------
 * Initializes system call vector with the appropriate service functions.
 * This function assigns various system calls to their corresponding service functions.
 * 
 * This is typically called at the start of phase 3.
 * 
 * No arguments.
 * 
 * returns:
 *     void
 */
void phase3_start_service_processes(void){
    systemCallVec[SYS_TERMINATE] = terminate;
    systemCallVec[SYS_WAIT] = wait;
    systemCallVec[SYS_SPAWN] = spawn;
    systemCallVec[SYS_SEMCREATE] = semCreate;
    systemCallVec[SYS_SEMP] = semP;
    systemCallVec[SYS_SEMV] = semV;
    systemCallVec[SYS_GETTIMEOFDAY] = getTimeOfDay;
    systemCallVec[SYS_GETPID] = getPid;
}


