#include <phase2.h>
#include <phase1.h>
#include <phase2.h>
#include <stdlib.h>
#include <string.h>
#include <phase1.h>



void phase3_init(void){
    systemCallVec[SYS_TERMINATE] = Terminate;
}

void phase3_start_service_processes(void){
    
}

int spawn(char *name, int (*func)(char *), char *arg, int stackSize, int priority, int *pid) {
    
}

void Terminate(int status) {

}