#include <phase3.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <phase4.h>
#include <phase2.h>
#include <phase1.h>

int CLOCK = 0;

struct sleepItem {
    int mboxId;
    int wakeupTime;
    struct sleepItem* next;
};

struct sleepItem sleepItems[MAXPROC];
struct sleepItem* sleepQueue;


void clock() {
    USLOSS_Console("clock called\n");
    while (1 == 1) {
        int status;
        waitDevice(CLOCK, 0, &status);

        /* Unblock each process that is ready to be unblocked*/
        if (sleepQueue != NULL) {
            struct sleepItem* cur = sleepQueue;
            while (cur != NULL ) {
                if (currentTime() >= sleepQueue->wakeupTime ) {
                    MboxSend( cur->mboxId, NULL, 0);
                    cur = cur->next;
                } else {
                    break;
                }
            }
        }
    }
    
}


void sleep(void* arg){
    USLOSS_Console("sleep called\n");

    USLOSS_Sysargs *args = (USLOSS_Sysargs*) arg;

    int pid = getpid();
    int mboxID = MboxCreate(0, 0);

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
        }
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

    MboxRecv(mboxID, NULL, 0);
    args->arg4 = 0;

    return;
}

void phase4_init(void){
    for (int i = 0; i < MAXPROC; i++) {
        sleepItems[i].mboxId = 0;
        sleepItems[i].wakeupTime = 0;
        sleepItems[i].next = NULL;
    }
}

void phase4_start_service_processes(void){
    systemCallVec[SYS_SLEEP] = sleep;
    spork("ClockD", clock, NULL, USLOSS_MIN_STACK, 1);
    
}
