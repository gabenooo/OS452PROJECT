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

struct sleepItem sleepQueue[MAXPROC];



void clock() {
    USLOSS_Console("clock called\n");
    while (1 == 1) {
        int status;
        int size;
        MboxRecv(CLOCK, &status, size);
        USLOSS_Console("recieved\n");
    }
    
}


void sleep(){
    
    return;
}

void phase4_init(void){
    for (int i = 0; i < MAXPROC; i++) {
        sleepQueue[i].mboxId = 0;
        sleepQueue[i].wakeupTime = 0;
        sleepQueue[i].next = NULL;
    }
    return;
}

void phase4_start_service_processes(void){
    systemCallVec[SYS_SLEEP] = sleep;
    //systemCallVec[CLOCK] = sleep;
    spork("ClockD", clock, NULL, USLOSS_MIN_STACK, 1);
    
}
