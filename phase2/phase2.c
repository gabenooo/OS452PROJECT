#include <phase2.h>
#include <phase1.h>
#include <stdlib.h>
#include <string.h>
#include <phase1.h>
/*
* File: phase2.c
* Authors: Kyle, Gabe Noriega
* Class: CSC 452 Russ Lewis
*
* Purpose: the purpose of this code is to simulate the mailbox system that allows
* us to handle interrupts in our simulation, as well as inter proc communication. 
*/


/*
* ENUM INTERRUPTS
* ---------------
* this is a enum that holds the mailbox ids according to device, used for handeling interrups
*/
enum INTERRUPTS {
    CLOCK = 0,
    DISK01 = 1,
    DISK02 = 2,
    TERM01 = 3,
    TERM02 = 4,
    TERM03 = 5,
    TERM04 = 6
};

/*
 * Struct:  shadowPCB
 * --------------------
 * This is the struct that is the shadow (or copy) of the process control block that uses a mailbox. It will hold feilds that correspond to
 * the actual PCB.
 * 
 * feilds:
 * pid - the process id number
 * priority - the priority of the process within a process table
 * status - this is an int that tell if a process is runable, running, or blocked
 * zombie - an int used to denote if a child or parent is a zombie process or not
 * number_of_children - an int used for debugging to see how many children a given process has
 * blocked - a bit to denote if a process is blocked or not 
 * struct shadowPCB* pNext - producer next pointer
 * struct shadowPCB* cNext - consumer next pointer
 */

struct shadowPCB {
    int pid; // if pid =0, proccess is dead
    int priority; 
    int status;
    int zombie;
    int number_of_children;
    int blocked;
    struct slot* slotToRead;


    //will need:
    struct shadowPCB* pNext;
    struct shadowPCB* cNext;
    
};

// shadowProcTable is the shadow table that holds the pcb copys
struct shadowPCB shadowProcTable[MAXPROC];

/*
 * Struct:  mailbox
 * --------------------
 * This is the struct that is the mailbox. It represents one mailbox slot in the mailboxes array. it holds the slot queue that
 * procs use to send messages
 * 
 * variables:
 * id - the mailbox id number
 * start - the start of the slot linked list
 * end - the end of the slot linked list
 * numSlots - the number of slots the mailbox can have
 * slotSize - the size of each slot, message size
 * slotsQueue - slotsQueue linked list
 * producerQueue- producer pointer queue
 * consumerQueue - consumer pointer queue
 */
struct mailbox {
    int id;
    struct slot * start; //linked list to not have contigus slots
    // head pointer 
    // pointer 
    struct slot * end;
    int numSlots;
    int numSlotsInUse;
    int slotSize;
    struct slot*      slotsQueue;
    struct shadowPCB* producerQueue;
    struct shadowPCB* consumerQueue;
};

/*
 * Struct:  slot
 * --------------------
 * This is the struct that represents a single slot where the messages get stored
 * 
 * variables:
 * inuse- int that represents if the slot is in use
 * msgSize - msg size
 * mailSlot - the actual message
 * nextSlot - the pointer to the next slot in the list
 */

struct slot{
    int inUse;
    int msgSize;
    char mailSlot[MAX_MESSAGE];
    struct slot * nextSlot;
};

static struct mailbox mailboxes[MAXMBOX];
static struct slot mailSlots[MAXSLOTS];

void (*systemCallVec[MAXSYSCALLS])(USLOSS_Sysargs *args);

int consumerQueued;
int curMailboxID;
int curSlotID;
int curTime;

/* Declaring helper functions here since we can't update the .h file*/
struct slot* getStartSlot();
int getNewId();
void termInteruptHandler(int _, void* payload);
void diskInteruptHandler(int _, void* payload);
void nullsys();


void phase2_start_service_processes(void){
    // Called by Phase 1 from init, once processes are running but before the testcase
    // begins. If your implementation requires any service processes to be running for
    // Phase 2 (I don’t expect that it will), then this is the place to call spork() to
    // create them.

    /* Creates the interupt mailboxes */
    MboxCreate(1, sizeof(int));
    MboxCreate(1, sizeof(int));
    MboxCreate(1, sizeof(int));
    MboxCreate(1, sizeof(int));
    MboxCreate(1, sizeof(int));
    MboxCreate(1, sizeof(int));
    MboxCreate(1, sizeof(int));

}

void phase2_clockHandler(void){
    // Called by Phase 1 from the clock interrupt. Use it to implement any logic
    // that you want to run every time that the clock interrupt occurs.
    if ( currentTime() - curTime >= 100000 ) {// && mailboxes[CLOCK].consumerQueue != NULL) {
        int status = 0;// = currentTime();
        //USLOSS_Console("CALLING device input; also cur time is : %d\n", currentTime());
        USLOSS_DeviceInput(USLOSS_CLOCK_DEV, 0, &status);
        //USLOSS_Console("status is %d\n", status);
        MboxCondSend(CLOCK, &status, sizeof(status));
        curTime = currentTime();
    }
}

void syscallHandler(int _, void *arg){
    USLOSS_Sysargs *args = (USLOSS_Sysargs*) arg;
    if (args->number >= MAXSYSCALLS || args->number < 0){
        USLOSS_Console("syscallHandler(): Invalid syscall number %d\n", args->number);
        USLOSS_Halt(1);
    } else {
        systemCallVec[args->number](arg);
    }
    
}

void phase2_init(void) {
    // Very similar to phase1_init(), this function is called by the testcase during
    // bootstrap, before any processes are running. Use it to initialize any data structures that you plan to use. 
    // You must not attempt to spork() any processes, or
    // use any other process-specific functions, since the processes are not yet running
    //USLOSS_IntVec[USLOSS_CLOCK_INT] = phase2_clockHandler; 
    USLOSS_IntVec[USLOSS_DISK_INT] = diskInteruptHandler;
    USLOSS_IntVec[USLOSS_TERM_INT] = termInteruptHandler;
    USLOSS_IntVec[USLOSS_SYSCALL_INT] = syscallHandler;
    /* Mailbox initialization */
    for (int i = 0; i < MAXMBOX; i++) {
        mailboxes[i].id = -1;
        mailboxes[i].start = NULL;
        mailboxes[i].end = NULL;
        mailboxes[i].numSlots = 0;
        mailboxes[i].numSlotsInUse = 0;
        mailboxes[i].slotSize = 0;
        mailboxes[i].producerQueue = NULL;
        mailboxes[i].consumerQueue = NULL;
        mailboxes[i].slotsQueue = NULL;

    }

    /* MailSlot initialization */
    for ( int i = 0; i < MAXSLOTS; i++) {    

        mailSlots[i].inUse = 0;
        mailSlots[i].msgSize = 0;
        for ( int j = 0; j < MAX_MESSAGE; j++) {
            mailSlots[i].mailSlot[j] = 0;
        }
        mailSlots[i].nextSlot = NULL;
    }
    curMailboxID = 0;
    curSlotID = 0;
    curTime = 0;
    consumerQueued = 0;

    memset(shadowProcTable, 0, sizeof(shadowProcTable));

    /* Set sysVec */
    for ( int i = 0; i < MAXSYSCALLS; i++ ) {
        systemCallVec[i] = &nullsys;
    }

}

void nullsys(USLOSS_Sysargs *args) {
    USLOSS_Console("nullsys(): Program called an unimplemented syscall.  syscall no: %d   PSR: 0x%02x\n", args->number, USLOSS_PsrGet());
    USLOSS_Halt(1);
}


// returns id of mailbox, or -1 if no more mailboxes, or -1 if invalid args
int MboxCreate(int slots, int slot_size){
    // slots is number of slots, slot_size is the message length for each slot?
    if (slots > MAXSLOTS) {
        return -1;
    }

    // first error check
    if ( (slots < 0|| slot_size < 0) || (slot_size > MAX_MESSAGE)) return -1;
    int newId = getNewId();
    // check if mailboxs is full: 
    if (newId == -1) return -1;

    // set up the mail box
    mailboxes[newId].id = newId;
    mailboxes[newId].slotSize = slot_size;
    mailboxes[newId].start = NULL;
    mailboxes[newId].numSlotsInUse = 0;
    mailboxes[newId].consumerQueue = NULL;
    mailboxes[newId].producerQueue = NULL;
    mailboxes[newId].end = mailboxes[newId].start;
    mailboxes[newId].numSlots = slots;

    return newId;
}


// returns 0 if successful, -1 if invalid arg
int MboxRelease(int mbox_id){
    // Destroys a mailbox. All slots consumed by the mailbox will be freed. All blocked
    // producers and consumers will be unblocked, and return -1.
    // Once the mailbox has been marked as destroyed, no more processes will be
    // allowed to block on it; any attempt to Send() or Recv() on it will return -1.
    // You must destroy the mailbox (and wake its blocked processes) promptly
    // but not necessarily instantly. By “promptly,” I mean that the various blocked
    // processes should be awoken, removed from any pending queues, etc. as soon
    // as is practical. However, we do not guarantee that all of these processes will
    // be awake when this function returns - and as such, it might not be possible to
    // re-create the mailbox immediately after this function returns.
    // Why might this happen? It depends on the mailbox implementation. In
    // your implementation, this might not be a problem - but in my implementation,
    // only one producer, and one consumer, can be waking up at a time - meaning
    // that it takes a while to “flush” any blocked producers and consumers from the
    // various queues.

    if (mailboxes[mbox_id].id < 0) {
        return -1;
    }

    mailboxes[mbox_id].id = -1;
    mailboxes[mbox_id].numSlots = 0;
    mailboxes[mbox_id].numSlotsInUse = 0;
    mailboxes[mbox_id].slotSize = 0;
    mailboxes[mbox_id].slotsQueue = NULL;

    // free each mail slot
    struct slot * cur = mailboxes[mbox_id].start;
    while (cur != NULL){
        cur->inUse = 0;
        for ( int j = 0; j < MAX_MESSAGE; j++) {
            cur->mailSlot[j] = 0;
        }
        struct slot * prev = cur;
        cur = cur->nextSlot;
        prev->nextSlot = NULL;
    }   

    //all blocked producers and consumers will be unblocked, and return -1
    struct shadowPCB* proCur = mailboxes[mbox_id].producerQueue;
    struct shadowPCB* conCur = mailboxes[mbox_id].consumerQueue;
    if (proCur != NULL || conCur != NULL){
        while (proCur != NULL){
            unblockProc(proCur->pid);
            proCur = proCur->pNext;
        }
        mailboxes[mbox_id].producerQueue = NULL;
        
        while (conCur != NULL){
            unblockProc(conCur->pid);
            conCur = conCur->cNext;
        }
        mailboxes[mbox_id].consumerQueue = NULL;
    }

    return 0;
}

int MboxSendHelper(int mbox_id, void *msg_ptr, int msg_size, int is_conditional) {
    struct slot* curSlot;

    /* Error checking */
    if (mailboxes[mbox_id].id < 0 || msg_size > mailboxes[mbox_id].slotSize) {
        return -1;
    }

    if (!(mailboxes[mbox_id].numSlots <= 0)) {
        curSlot = getStartSlot();
        if (curSlot == NULL) {
            return -2;
        }
    }

    //USLOSS_Console("Mailbox is %d and numSlots in use is %d and number of slots is %d\n", mbox_id, mailboxes[mbox_id % MAXMBOX].numSlotsInUse, mailboxes[mbox_id % MAXMBOX].numSlots);
    //USLOSS_Console("Mailbox is %d and consumer queue ID is %p\n", mbox_id, mailboxes[mbox_id % MAXMBOX].consumerQueue);

    if (mailboxes[mbox_id % MAXMBOX].numSlots == 0 && mailboxes[mbox_id % MAXMBOX].consumerQueue != NULL) {
        goto ConsumerQueue;
    }

    if ( (mailboxes[mbox_id % MAXMBOX].numSlotsInUse >= mailboxes[mbox_id % MAXMBOX].numSlots) ) {
        int QueProcID = getpid();
        shadowProcTable[QueProcID % MAXPROC].blocked = 1;
        shadowProcTable[QueProcID % MAXPROC].pid = QueProcID;

        struct shadowPCB* cur = mailboxes[mbox_id].producerQueue;
        
        /* Block process until space is available, if mailbox destroyed then return -2 */
        if ( is_conditional == 1 ) {
            return -2;
        } else {
            if (cur == NULL){
                //USLOSS_Console("should be null\n");
                mailboxes[mbox_id].producerQueue = &shadowProcTable[QueProcID % MAXPROC];
            } else {
                
                while (cur->pNext != NULL){
                    //USLOSS_Console("%d\n", cur->pid);
                    cur = cur->pNext;
                }
                cur->pNext = &shadowProcTable[QueProcID % MAXPROC];
            }
            //USLOSS_Console("BLOCKING ME %d %d %d\n", QueProcID, shadowProcTable[QueProcID].pid, mailboxes[mbox_id].producerQueue->pid);
            blockMe(98);
            //USLOSS_Console("UNBLOCKING ME \n");

        }
        if (mailboxes[mbox_id].id < 0) { return -1; }
        mailboxes[mbox_id].producerQueue = mailboxes[mbox_id].producerQueue->pNext;  
        if (mailboxes[mbox_id].numSlots <= 0 ) { return 0; }
    }

    /* For non empty mailboxes add the memssage to the queue*/
    if (mailboxes[mbox_id].numSlots > 0) {
        curSlot->inUse = 1;
        curSlot->msgSize = msg_size;
        if(msg_ptr != NULL) { strcpy(curSlot->mailSlot, msg_ptr); }

        /* Adds the message to the message queue */
        if (mailboxes[mbox_id % MAXMBOX].slotsQueue == NULL) {
            mailboxes[mbox_id % MAXMBOX].slotsQueue = curSlot;
        } else {
            struct slot* next = mailboxes[mbox_id % MAXMBOX].slotsQueue;
            while (next->nextSlot != NULL) {
                next = next->nextSlot;
            }
            next->nextSlot = curSlot;
        }
        mailboxes[mbox_id % MAXMBOX].numSlotsInUse++;
    }

    /* If the consumer is waiting, unblock them and remove them from the consumer queue */
    ConsumerQueue:
    if (mailboxes[mbox_id % MAXMBOX].consumerQueue != NULL) {
        
        mailboxes[mbox_id % MAXMBOX].slotsQueue = curSlot;
        int pid = mailboxes[mbox_id % MAXMBOX].consumerQueue->pid;
        mailboxes[mbox_id % MAXMBOX].consumerQueue = mailboxes[mbox_id % MAXMBOX].consumerQueue->cNext;

        /* Unblock the process */
        //USLOSS_Console("Unblocking\n");
        if (mailboxes[mbox_id].numSlots > 0) {
            if (mailboxes[mbox_id].slotsQueue->mailSlot != NULL) { 
                shadowProcTable[pid % MAXPROC].slotToRead = mailboxes[mbox_id].slotsQueue;
            }
            /* Removes message from slot queue */
            mailboxes[mbox_id % MAXMBOX].numSlotsInUse--;
            mailboxes[mbox_id].slotsQueue = mailboxes[mbox_id].slotsQueue->nextSlot;
        }

        unblockProc(pid);

    } else if (mailboxes[mbox_id].numSlots <= 0) {
        if ( is_conditional == 1 ) {
            return -2;
        } else {
            //mUSLOSS_Console("blocking\n");
            blockMe(98);
            //USLOSS_Console("UNBLOCKING\n");

        }
        goto ConsumerQueue;
    }

    return 0;


}

// returns 0 if successful, -1 if invalid args
int MboxSend(int mbox_id, void *msg_ptr, int msg_size){
    return MboxSendHelper(mbox_id, msg_ptr, msg_size, 0);
}


// returns size of received msg if successful, -1 if invalid args
int MboxRecv(int mbox_id, void *msg_ptr, int msg_max_size){
    // Waits to receive a message through a mailbox. If there is a message already
    // queued in a mail slot, it may read it directly and return (but be careful to obey
    // the ordering rules we discussed earlier in the spec). Otherwise it will block until
    // a message is available. (But note the special rules for zero-slot mailboxes, see
    // above.)
    int msgSize = 0;
    
    // todo: error checking (RETURN -1)
    if (mbox_id < 0 || mbox_id > MAXMBOX || mailboxes[mbox_id].id < 0) {
        return -1;
    } // then check buffer len



    if (mailboxes[mbox_id].numSlots == 0 && mailboxes[mbox_id].producerQueue != NULL) {
        
        unblockProc(mailboxes[mbox_id].producerQueue->pid);
        return 0;
    }

    if (mailboxes[mbox_id].slotsQueue != NULL){
        // a message is waiting so we copy it over
        msgSize = mailboxes[mbox_id].slotsQueue->msgSize;
        if (mailboxes[mbox_id].slotsQueue->mailSlot != NULL) { 
            if (mailboxes[mbox_id].slotsQueue->msgSize > msg_max_size) {
                return -1;
            }
            strcpy(msg_ptr, mailboxes[mbox_id].slotsQueue->mailSlot);
        }
        /* Removes message from slot queue */
        mailboxes[mbox_id % MAXMBOX].numSlotsInUse--;
        mailboxes[mbox_id].slotsQueue = mailboxes[mbox_id].slotsQueue->nextSlot;
        /* Removes a producer if present from the producer queue */
        if (mailboxes[mbox_id % MAXMBOX].producerQueue != NULL) { 
            //USLOSS_Console("cur proc is %d, UNBLOCKING %d\n", getpid(), mailboxes[mbox_id % MAXMBOX].producerQueue->pid);
            if (shadowProcTable[mailboxes[mbox_id % MAXMBOX].producerQueue->pid].blocked == 1) {
                shadowProcTable[mailboxes[mbox_id % MAXMBOX].producerQueue->pid].blocked = 0;
                unblockProc(mailboxes[mbox_id % MAXMBOX].producerQueue->pid);  
            }
               
        }

        return msgSize;
    } else {
        // queue up proc and block
        
        int QueProcID = getpid();
        shadowProcTable[QueProcID % MAXPROC].blocked = 1;
        shadowProcTable[QueProcID % MAXPROC].pid = QueProcID;

        struct shadowPCB* cur = mailboxes[mbox_id].consumerQueue;
        if (cur == NULL){
            mailboxes[mbox_id].consumerQueue = &shadowProcTable[QueProcID % MAXPROC];
        } else {
            while (cur->cNext != NULL){
                cur = cur->cNext;
            }
            cur->cNext = &shadowProcTable[QueProcID % MAXPROC];
        }
        blockMe(99);

        if (shadowProcTable[getpid() % MAXPROC].slotToRead == NULL || shadowProcTable[getpid() % MAXPROC].slotToRead->msgSize > msg_max_size) {
            return -1;
        }

        /* Removes a producer if present from the producer queue */
        if (mailboxes[mbox_id].id < 0) { return -1; }

 
        msgSize = shadowProcTable[getpid() % MAXPROC].slotToRead->msgSize;
        if (shadowProcTable[getpid() % MAXPROC].slotToRead->msgSize > 0) {
            strcpy(msg_ptr, shadowProcTable[getpid() % MAXPROC].slotToRead->mailSlot);
        }
        

        /* Removes message from slot queue */
        shadowProcTable[getpid() % MAXPROC].slotToRead = NULL;


        /* Removes a producer if present from the producer queue */
        if (mailboxes[mbox_id % MAXMBOX].producerQueue != NULL) { 
            if (shadowProcTable[mailboxes[mbox_id % MAXMBOX].producerQueue->pid].blocked == 1) {
                shadowProcTable[mailboxes[mbox_id % MAXMBOX].producerQueue->pid].blocked = 0;
                unblockProc(mailboxes[mbox_id % MAXMBOX].producerQueue->pid);  
            }    
        }
        
        return msgSize; 
            
    }

    return 0;
}


// returns 0 if successful, 1 if mailbox full, -1 if illegal args
int MboxCondSend(int mbox_id, void *msg_ptr, int msg_size){
    return MboxSendHelper(mbox_id, msg_ptr, msg_size, 1);
}


// returns 0 if successful, 1 if no msg available, -1 if illegal args
int MboxCondRecv(int mbox_id, void *msg_ptr, int msg_max_size){
    // These functions work exactly like their non-Cond versions, except that they
    // refuse to block. If, at any point, they would normally have to block, they will
    // return -2 instead.
    // While these functions will never block, they might context switch, if they
    // wake up a process that was higher priority than the one on which the interrupt
    // handler is running. This is not a problem.
    // Note that you may find it useful, instead of implementing two different copies
    // of Send() and Recv(), to instead create (private) helper functions, which both
    // the Cond and non-Cond versions of your functions can call. But remember: you
    // must not change the declaration of any function called by the testcases!
    int msgSize = 0;
    // todo: error checking (RETURN -1)
    if (mbox_id < 0 || mbox_id > MAXMBOX){
        return -1;
    } // then check buffer len

    // USLOSS_Console("--Starting----\n");
    // struct slot* curSlot = mailboxes[mbox_id].slotsQueue;
    // while(curSlot != NULL) {
    //     USLOSS_Console("----%s----\n", curSlot->mailSlot);
    //     curSlot = curSlot->nextSlot;
    // }
    // USLOSS_Console("--Ending----\n");


    if (mailboxes[mbox_id].slotsQueue != NULL){
        // a message is waiting so we copy it over
        msgSize = mailboxes[mbox_id].slotsQueue->msgSize;
        if (mailboxes[mbox_id].slotsQueue->mailSlot != NULL) { 
            strcpy(msg_ptr, mailboxes[mbox_id].slotsQueue->mailSlot);
        }

        /* Removes message from slot queue */
        struct slot* curSlot = mailboxes[mbox_id].slotsQueue;
        mailboxes[mbox_id].slotsQueue = mailboxes[mbox_id].slotsQueue->nextSlot;
        mailboxes[mbox_id % MAXMBOX].numSlotsInUse--;
        curSlot->inUse = 0;
        curSlot->nextSlot = NULL;
        for ( int j = 0; j < MAX_MESSAGE; j++) {
            curSlot->mailSlot[j] = 0;
        }
        
        /* Removes a producer if present from the producer queue */
        if (mailboxes[mbox_id % MAXMBOX].producerQueue != NULL) { 
            unblockProc(mailboxes[mbox_id % MAXMBOX].producerQueue->pid);     
        }

        return msgSize;
    } else {
        return -2;
    }
    return 0;
}


// type = interrupt device type, unit = # of device (when more than one),
// status = where interrupt handler puts device's status register.
void waitDevice(int type, int unit, int *status){
    // Waits for an interrupt to fire, on a given device. Only three device types are
    // valid: the clock, disk, and terminal devices. The unit field must be a valid value
    // (0 for clock; 0,1 for disk; 0,1,2,3 for terminal); if it is not, report an error and
    // halt the simulation.
    // This function will Recv() from the proper mailbox for this device; when the
    // message arrives, it will store the status (remember, the status was sent, as the
    // message payload, from the interrupt handler) into the out parameter and then
    // return.

    if (type == 0){
        MboxRecv(CLOCK, status, sizeof(int));
    } else if (type == USLOSS_DISK_DEV){
        switch (unit) {
            case 0:
                MboxRecv(DISK01, status, sizeof(int));
                break;
            case 1:
                MboxRecv(DISK02, status, sizeof(int));
                break;
            default:
                USLOSS_Halt(0);
        }
    } else if (type == USLOSS_TERM_DEV){
        switch (unit) {
            case 0:
                MboxRecv(TERM01, status, sizeof(int));
                break;
            case 1:
                MboxRecv(TERM02, status, sizeof(int));
                break;
            case 2:
                MboxRecv(TERM03, status, sizeof(int));
                break;
            case 3:
                MboxRecv(TERM04, status, sizeof(int));
                break;
            default:
                USLOSS_Halt(0);// throw error halt
        }
    } else {
        USLOSS_Halt(0);
    }
}

void wakeupByDevice(int type, int unit, int status){
    //????????
}

/******************** INTERUPT HANDLERS ********************/

/* Handles interups for terminal */
void termInteruptHandler(int _, void* payload) {
    

    int unit = (int)(long)payload;
    int status = 0;
    USLOSS_DeviceInput(USLOSS_TERM_DEV, unit, &status);
    int termDev = -1;
    if (unit == 0) {
        termDev = TERM01;
    } else if (unit == 1) {
        termDev = TERM02;
    } else if (unit == 2) {
        termDev = TERM03;
    } else if (unit == 3) {
        termDev = TERM04;
    }

    MboxCondSend(termDev, &status, sizeof(status));
}

/* Handles interupts for disk */
void diskInteruptHandler(int _, void* payload) {
    int unit = (int)(long)payload;
    int status = 0;
    USLOSS_DeviceInput(USLOSS_TERM_DEV, unit, &status);

    int diskDec = -1;
    if (unit == 0) {
        diskDec = DISK01;
    } else if (unit == 1) {
        diskDec = DISK02;
    }

    MboxCondSend(diskDec, &status, sizeof(status));
}


/******************** ALL THE HELPER FUNCTIONS ********************/

/* Gets an empty mailbox ID, returns -1 if full */
int getNewId() {
    // return.getStartSlot
    for (int i = 0; i < MAXMBOX; i++) {
        if (mailboxes[i].id == -1) {
            curMailboxID = i;
            return i;
        }
    }

    return -1;
}



/* Returns the index of the start slot for the series of slots requested */
struct slot* getStartSlot() {
    
    for ( int i = curSlotID + 1; i < MAXSLOTS + curSlotID + 1; i++ ) {
        if ( mailSlots[i % MAXSLOTS].inUse == 0 ) {
            curSlotID = i % MAXSLOTS;
            return &mailSlots[i % MAXSLOTS];
        }
    }

    /* If no slots available return -1 */
    return NULL;
}