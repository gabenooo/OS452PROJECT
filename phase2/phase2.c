#include <phase2.h>

#include <stdlib.h>


//systemCallVec;

//slots with size MAX_MESSAGE

//shadow proccess table

struct mailbox {
    int id;
    struct slot * start; //linked list to not have contigus slots
    // head pointer 
    // pointer 
    struct slot * end;
    int numSlots;

    //not sure if we need this:
    struct mailSlot* nextMailBox;

    //struct mailSlot*;
};

//queue of producer and consumer, 
// can get pid when they call mailbox send we call call getPID -> then use that to make a copy of the proccess table entry in our shadow table


struct slot{
    int inUse;
    int slotSize;
    char mailSlot[MAX_MESSAGE];
    struct slot * nextSlot;
};

static struct mailbox mailboxes[MAXMBOX];

static struct slot mailSlots[MAXSLOTS];

int curMailboxID;
int curSlotID;

void phase2_start_service_processes(void){
    // Called by Phase 1 from init, once processes are running but before the testcase
    // begins. If your implementation requires any service processes to be running for
    // Phase 2 (I don’t expect that it will), then this is the place to call spork() to
    // create them.

}


void phase2_clockHandler(void){
    // Called by Phase 1 from the clock interrupt. Use it to implement any logic
    // that you want to run every time that the clock interrupt occurs.
}


void phase2_init(void){
    // Very similar to phase1_init(), this function is called by the testcase during
    // bootstrap, before any processes are running. Use it to initialize any data structures that you plan to use. 
    // You must not attempt to spork() any processes, or
    // use any other process-specific functions, since the processes are not yet running

    /* Mailbox initialization */
    for (int i = 0; i < MAXMBOX; i++) {
        mailboxes[i].id = -1;
        mailboxes[i].start = 0;
        mailboxes[i].end = 0;
        mailboxes[i].cur = NULL;
        mailboxes[i].nextMailBox = NULL;
    }

    /* MailSlot initialization */
    for ( int i = 0; i < MAXSLOTS; i++) {
        mailSlots[i].inUse = 0;
        mailSlots[i].slotSize = 0;
        for ( int j = 0; j < MAX_MESSAGE; j++) {
            mailSlots[i].mailSlot[j] = 0;
        }
    }

    curMailboxID = 0;
    curSlotID = 0;
}


// returns id of mailbox, or -1 if no more mailboxes, or -1 if invalid args
int MboxCreate(int slots, int slot_size){
    // slots is number of slots, slot_size is the message length for each slot?

    // first error check
    if ( (slots < 0|| slot_size < 0) || (slot_size > MAX_MESSAGE)) return -1;
    int newId = getNewId();
    // check if mailbox is full: 
    if (newId == -1) return -1;
    
    mailboxes[newId].id = newId;
    mailboxes[newId].start = getStartSlot(slots);
    mailboxes[newId].end = mailboxes[newId].start + slots;
    mailboxes[newId].cur = &mailSlots[mailboxes[newId].start];
    mailboxes[newId].nextMailBox = NULL;
    mailboxes[newId].numSlots = slots;
    
    //set cur mailbox slots to in use
    for (int i = mailboxes[newId].start; i < mailboxes[newId].end; i++){
        mailSlots[i].inUse = 1;
        mailSlots[i].slotSize = slot_size;
        mailSlots[i].mailSlot = NULL;
    }

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
// struct mailbox {
//     int id;
//     int start;
//     int end;
//     struct slot* cur;
    
//     struct mailSlot* nextMailBox;

// }
    // free each mail slot (might have to change this later)
    for (int i = mailboxes[mbox_id].start; i < mailboxes[mbox_id].end; i++){
        mailSlots[i].inUse = 0;
        mailSlots[i].slotSize = 0;
        mailSlots[i].mailSlot = NULL;
    }


    return 0;
}


// returns 0 if successful, -1 if invalid args
int MboxSend(int mbox_id, void *msg_ptr, int msg_size){
    // Sends a message through a mailbox. If the message is delivered directly to a
    // consumer or queued up in a mail slot, then this function will not block (although
    // it might context switch, even then, if it wakes up a higher-priority process).
    // If there are no consumers queued and no space available to queue a message,
    // then this process will block until the message can be delivered - either to a
    // consumer, or into a mail slot.
    return 0;
}


// returns size of received msg if successful, -1 if invalid args
int MboxRecv(int mbox_id, void *msg_ptr, int msg_max_size){
    // Waits to receive a message through a mailbox. If there is a message already
    // queued in a mail slot, it may read it directly and return (but be careful to obey
    // the ordering rules we discussed earlier in the spec). Otherwise it will block until
    // a message is available. (But note the special rules for zero-slot mailboxes, see
    // above.)
    return 0;
}


// returns 0 if successful, 1 if mailbox full, -1 if illegal args
int MboxCondSend(int mbox_id, void *msg_ptr, int msg_size){
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
    return 0;
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
}
void wakeupByDevice(int type, int unit, int status){
    //????????
}

// 
extern void (*systemCallVec[])(USLOSS_Sysargs *args);

/******************** ALL THE HELPER FUNCTIONS ********************/

/* Gets an empty mailbox ID, returns -1 if full */
int getNewID() {
    for (int i = curMailboxID + 1; i < MAXMBOX; i++) {
        if (mailboxes[i].id == -1) {
            curMailboxID = i;
            return i;
        }
    }

    return -1;
}

/* Returns the index of the start slot for the series of slots requested */
int getStartSlot(int numOfSlots) {
    int startSlot = curSlotID + 1;
    int slotCounter = 0;

    /* Loop over each slot, starting at the current slot index */
    for ( int i = startSlot; i < MAXSLOTS + startSlot; i++) {
        /* If we've encountered a slot in-use, reset the counter and continue on */
        if ( mailSlots[i % MAXSLOTS].inUse == 1 ) {
            startSlot = -1;
            slotCounter = 0;
            continue;
        }

        /* If we have previously reset, set the start and count this slot, otherwise count the slot */
        if ( startSlot == -1 ) {
            startSlot = i % MAXSLOTS;
            slotCounter++;
        }
        else {
            slotCounter++;
        }

        /* If all these slots are free, return the starting slot number */
        if ( slotCounter == numOfSlots) {
            curSlotID = startSlot + numOfSlots;
            return startSlot;
        }
    }
    /* If no combination of slots available return -1 */
    return -1;
}