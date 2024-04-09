#include <phase3.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <phase4.h>
#include <phase2.h>
#include <phase1.h>

int CLOCK = 0;



int term1Mut = -1;
int term2Mut = -1;
int term3Mut = -1;
int term4Mut = -1;


struct sleepItem {
    int mboxId;
    int wakeupTime;
    struct sleepItem* next;
};

struct sleepItem sleepItems[MAXPROC];
struct sleepItem* sleepQueue;


void termRead(void* arg) {
    USLOSS_Sysargs *args = (USLOSS_Sysargs*) arg;
    // CHECKMODE;
    void * buffer = args->arg1;
    long bufferSize = args->arg2;
    long unitID = args->arg3;

    int status;
    USLOSS_Console("Calling read\n");

    USLOSS_DeviceInput(USLOSS_TERM_DEV, unitID, &status);

    USLOSS_Console("status of read is %d\n", status);
    

    switch (unitID){
        case 1:          
            break;
        case 2:
            break;
        case 3:
            break;
        case 4:
            break;
        default:
            break;

    }
    long numsToRead = -1;
    args->arg2 = numsToRead;

}

void termWrite(void* arg) {
    USLOSS_Sysargs *args = (USLOSS_Sysargs*) arg;
    USLOSS_Console("Write called\n");
}


void clock() {
    //USLOSS_Console("clock called\n");
    while (1 == 1) {
        int status;
        waitDevice(CLOCK, 0, &status);

        /* Unblock each process that is ready to be unblocked*/
        if (sleepQueue != NULL) {
            struct sleepItem* cur = sleepQueue;
            while (cur != NULL ) {
                if (currentTime() > cur->wakeupTime ) {
                    MboxSend( cur->mboxId, NULL, 0);
                    cur = cur->next;
                    sleepQueue = cur;
                } else {
                    break;
                }
            }
        }
    }
}


void sleep(void* arg){
    //USLOSS_Console("sleep called\n");

    USLOSS_Sysargs *args = (USLOSS_Sysargs*) arg;

    int pid = getpid();
    int mboxID = MboxCreate(0, 0);

    int curTime = currentTime();
    //USLOSS_Console("time is %d, end time is %d\n", curTime, (curTime + ((int)args->arg1 * 1000000)));

    sleepItems[pid].mboxId = mboxID;
    sleepItems[pid].wakeupTime = (currentTime() + ((int)args->arg1 * 1000000));

    /* First checks if queue is currently null */
    if (sleepQueue == NULL) {
        sleepQueue = &sleepItems[pid];
    } else {
        /* Otherwise, if the new item is quicker than current, add to front of queue */
        struct sleepItem* cur = sleepQueue;
        if (sleepItems[pid].wakeupTime < cur->wakeupTime) {
            sleepItems[pid].next = cur;
            sleepQueue = &sleepItems[pid];
        } else {
            /* Otherwise insert where it belongs in the queue */
            while (cur->next != NULL) {
                if (sleepItems[pid].wakeupTime < cur->next->wakeupTime ) {
                    sleepItems[pid].next = cur->next;
                    cur->next = &sleepItems[pid];
                    break;
                } 
                cur = cur->next;
            }
            /* If no position found then add to the end of the queue */
            if (cur->next == NULL) {
                cur->next = &sleepItems[pid];
            }
        }
    }

    MboxRecv(mboxID, NULL, 0);
    args->arg4 = 0;

    return;
}

void termd(char* arg){
    int termNum =  atoi(arg);

    while (1 == 1) {
        int status;
        waitDevice(USLOSS_TERM_DEV, termNum, &status);
        USLOSS_Console("terminal interupt\n");

    }

}

void phase4_init(void){
    for (int i = 0; i < MAXPROC; i++) {
        sleepItems[i].mboxId = 0;
        sleepItems[i].wakeupTime = 0;
        sleepItems[i].next = NULL;
    }
}

void phase4_start_service_processes(void){
    term1Mut = MboxCreate(0, 0);
    term2Mut = MboxCreate(0, 0);
    term3Mut = MboxCreate(0, 0);
    term4Mut = MboxCreate(0, 0);

    systemCallVec[SYS_SLEEP] = sleep;
    systemCallVec[SYS_TERMREAD] = termRead;
    systemCallVec[SYS_TERMWRITE] = termWrite;
    spork("ClockD", clock, NULL, USLOSS_MIN_STACK, 1);

    spork("Term1D", termd, "0", USLOSS_MIN_STACK, 1);
    spork("Term2D", termd, "1", USLOSS_MIN_STACK, 1);
    spork("Term3D", termd, "2", USLOSS_MIN_STACK, 1);
    spork("Term4D", termd, "3", USLOSS_MIN_STACK, 1);

    int ctrReg = 0x6;
    USLOSS_DeviceOutput(USLOSS_TERM_DEV, 0, ctrReg);
    USLOSS_DeviceOutput(USLOSS_TERM_DEV, 1, ctrReg);
    USLOSS_DeviceOutput(USLOSS_TERM_DEV, 2, ctrReg);
    USLOSS_DeviceOutput(USLOSS_TERM_DEV, 3, ctrReg);
    
}
