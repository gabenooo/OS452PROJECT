#include <phase3.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <phase4.h>
#include <phase2.h>
#include <phase1.h>

struct sleepItem {
    int procId;
    int mboxId;
    int wakeupTime;
    struct sleepItem next;
};

struct sleepItem sleepQueue[MAXPROC];

void phase4_init(void){

    return;
}


void sleep(){
    
    return;
}

void phase4_start_service_processes(void){
    systemCallVec[SYS_SLEEP] = sleep;
    
}
