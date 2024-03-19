#include <phase2.h>
#include <phase1.h>
#include <phase2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <phase1.h>



int spawn(char *name, int (*func)(char *), char *arg, int stackSize, int priority, int *pid) {
    printf("eee\n");
    
}

void terminate(int status) {
    int pid = getpid();

    //while (join())
}

void phase3_init(void){
    systemCallVec[SYS_TERMINATE] = terminate;
    systemCallVec[SYS_SPAWN] = spawn;
}

void phase3_start_service_processes(void){
    
}



