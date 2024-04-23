#include <phase3.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <phase4.h>
#include <phase2.h>
#include <phase1.h>

int CLOCK = 0;

/*
 * termMut array for the ternimal mailbox locks
 */
int termMut[4];
/*
 * termsender array for the mailboxs that send one char at a time to the terminal procs
 */
int termSender[4];

int writeCompleted[4];

int diskTracks[2];

/*
 * termRecvMbox array for the ternimal mailbox used for recv
 */
int termRecvMbox[4];
/*
 * buffers for terminal recv
 */
char buffers[4][MAXLINE + 1];

/*
 * Struct:  sleepItem
 * --------------------
 * This is the struct that holds a mailboxId used to wake up a proc
 * 
 * variables:
 * mboxId- int that is the id of the mailbox to send to
 * wakeupTime - int that is the time to wake up the proc
 * next - the next sleepItem in the list
 */

struct sleepItem {
    int mboxId;
    int wakeupTime;
    struct sleepItem* next;
};

struct diskInfo {
    int mboxId;
    int first;
    struct diskInfo* next;
};

struct diskInfo disks[MAXPROC];
struct diskInfo *diskQueue;

/*
 * sleepItems struct for sleeping procs
 */
struct sleepItem sleepItems[MAXPROC];
/*
 * pointer to wake up procs sleeping
 */
struct sleepItem* sleepQueue;

void diskSize(void* arg) {
    USLOSS_Sysargs *args = (USLOSS_Sysargs*) arg;
    int response;
    long unitID = args->arg1;

    USLOSS_DeviceRequest req;
    req.opr = USLOSS_DISK_TRACKS;
    req.reg1 = &response;

    USLOSS_DeviceOutput(USLOSS_DISK_DEV, unitID, &req);
    MboxRecv(diskTracks[unitID], NULL, 0);  

    args->arg1 = 512;
    args->arg2 = 16;
    args->arg3 = response;

    //USLOSS_Console("UnitID is %d and size is %d\n", unitID, response);
}

void diskRead(void* arg) {
    USLOSS_Sysargs *args = (USLOSS_Sysargs*) arg;
    void * buffer = args->arg1;
    long sectors = args->arg2;
    long track = args->arg3;
    long first = args->arg3;
    long unit = args->arg3;

    int mbox = MboxCreate(0,0);

    int diskIndex = getpid();

    disks[diskIndex].mboxId = mbox;
    disks[diskIndex].first = first;

    appendQueue(*disks[diskIndex]);
    // if first go
    if (diskQueue[0].next = NULL){
        // only one in queue
        USLOSS_DeviceRequest req;
        req.opr = USLOSS_DISK_WRITE;
        //req.reg1 = (void*)(long)blockIndex;
        req.reg2 = buffer;
        USLOSS_DeviceOutput(USLOSS_DISK_DEV, unit, &req);

        
    } else {
        MboxRecv(mbox, NULL, NULL);

        //read
        USLOSS_DeviceRequest req;
        req.opr = USLOSS_DISK_WRITE;
        //req.reg1 = (void*)(long)blockIndex;
        req.reg2 = buffer;
        USLOSS_DeviceOutput(USLOSS_DISK_DEV, unit, &req);
        // then send on next mailbox
    }

    // unblock next in queue
    if (diskQueue[0].next != NULL){
            MboxSend(diskQueue[0].mboxId, NULL, NULL);
        }
    
}


void diskWrite(void* arg) {
    USLOSS_Sysargs *args = (USLOSS_Sysargs*) arg;
    void * buffer = args->arg1;
    long sectors = args->arg2;
    long track = args->arg3;
    long first = args->arg3;
    long unit = args->arg3;
    USLOSS_DeviceRequest req;

    /* Initialize the write mailbox */
    int diskIndex = getpid();
    disks[diskIndex].mboxId = MboxCreate(0,0);
    disks[diskIndex].first = first;
    
    /* Add to the queue and wait if needed */
    appendQueue(&disks[diskIndex]);
    if (diskQueue->next != NULL) {
        MboxRecv(disks[diskIndex].mboxId, NULL, 0);
    }

    /* Perform the operations */
    for (int i = 0; i < sectors; i++) {
        seek(first, unit);
        USLOSS_DeviceOutput(USLOSS_DISK_DEV, unit, &req);
        MboxRecv(diskTracks[unit], NULL, 0);  
    }


    diskQueue = diskQueue->next;




}


/*
 * Function:  termRead
 * -------------------
 * This function reads data from a specified terminal into a buffer.
 * It checks for errors, reads from the terminal receive mailbox into a temporary buffer,
 * and then copies the appropriate amount of data to the provided buffer.
 * It sets the actual length of data read and returns appropriate values.
 * 
 * arguments:
 *  void* arg - arguments passed to the function (USLOSS_Sysargs structure)
 * 
 * returns:
 *  void
 */
void termRead(void* arg) {
    USLOSS_Sysargs *args = (USLOSS_Sysargs*) arg;
    void * buffer = args->arg1;
    long bufferSize = args->arg2;
    long unitID = args->arg3;

    /* Error Checking */
    if (bufferSize <= 0 || unitID < 0 || unitID > 3) {
        args->arg4 = -1;
        return;
    }

    /* Read into a buffer first */
    char fullBuffer[MAXLINE];
    memset(fullBuffer, '\0', MAXLINE);

    int status = MboxRecv(termRecvMbox[unitID], fullBuffer, MAXLINE+1);
    strncpy(buffer, fullBuffer, bufferSize);

    /* Set the out parameters */
    int actualLen = strlen(buffer);
    if (actualLen < bufferSize) {
        args->arg2 = actualLen;
    } else {
        args->arg2 = bufferSize;
    }
    args->arg4 = 0;
}

/*
 * Function:  termWrite
 * --------------------
 * This function writes data to a specified terminal.
 * It acquires the terminal mutex, then sends each character in the buffer
 * to the terminal sender mailbox for transmission.
 * After sending each character, it waits for the write completion signal.
 * Finally, it releases the terminal mutex.
 * 
 * arguments:
 *  void* arg - arguments passed to the function (USLOSS_Sysargs structure)
 * 
 * returns:
 *  void
 */
void termWrite(void* arg) {
    USLOSS_Sysargs *args = (USLOSS_Sysargs*) arg;
    void * buffer = args->arg1;
    long bufferSize = args->arg2;
    long unitID = args->arg3;

    /* Grab the lock */
    MboxSend(termMut[unitID], NULL, NULL);

    /* Send the message */
    for (int c = 0 ; c < bufferSize; c++){
        char* toSend = &buffer[c];
        MboxSend(termSender[unitID], toSend,1);
        MboxRecv(writeCompleted[unitID], NULL, 0);
    }
    /* Release the lock */
    MboxRecv(termMut[unitID], NULL, NULL);
}

/*
 * Function:  clock
 * ----------------
 * This function serves as the clock device handler.
 * It continuously waits for clock device interrupts.
 * When an interrupt occurs, it checks the sleep queue to unblock any processes
 * that are ready to be woken up based on their wakeup time.
 * 
 * arguments:
 *  None
 * 
 * returns:
 *  void
 */
void clock() {
    /* Loops indefinetly */
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

/*
 * Function:  sleep
 * ----------------
 * This function puts the current process to sleep for a specified amount of time.
 * It creates a mailbox for the process, calculates the wakeup time based on the
 * current time and the provided sleep duration, and adds the process to the sleep queue.
 * The process then waits on its mailbox until it is woken up.
 * 
 * arguments:
 *  void* arg - arguments passed to the function (USLOSS_Sysargs structure)
 * 
 * returns:
 *  void
 */
void sleep(void* arg){
    USLOSS_Sysargs *args = (USLOSS_Sysargs*) arg;

    int pid = getpid();
    int mboxID = MboxCreate(0, 0);

    int curTime = currentTime();

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
}

/*
 * Function:  termd
 * ----------------
 * This function handles terminal input and output for a specific terminal.
 * It continuously waits for input or output requests on the specified terminal device.
 * When input is received, it stores characters in the buffer and sends it to the
 * terminal receive mailbox when the buffer is full or a newline character is encountered.
 * When output is requested, it sends characters from the terminal sender mailbox to
 * the terminal device for transmission.
 * 
 * arguments:
 *  char* arg - argument indicating the terminal number
 * 
 * returns:
 *  void
 */
void termd(char* arg){
    int termNum =  atoi(arg);

    while (1 == 1) {
        int status;
        waitDevice(USLOSS_TERM_DEV, termNum, &status);

        int recvStatus = USLOSS_TERM_STAT_RECV(status);
        int xmitStatus = USLOSS_TERM_STAT_XMIT(status);

        /* We are ready to recieve a character */
        if (recvStatus == USLOSS_DEV_BUSY) {
            char readChar = USLOSS_TERM_STAT_CHAR(status);
            size_t len = strlen(buffers[termNum]);
            buffers[termNum][len] = readChar;
            buffers[termNum][len + 1] = '\0';

            /* Checks if buffer is full or newline encounterd */
            if (len + 1 >= MAXLINE || readChar == '\n') {
                int mboxstatus = MboxCondSend(termRecvMbox[termNum], buffers[termNum], len+1);
                resetBuffer(termNum);
            }
        }

        /* We are ready to send a character */
        if (xmitStatus == USLOSS_DEV_READY) {
            char toSend[1];
            int cr_val;
            int xmitRecStatus = MboxCondRecv(termSender[termNum], toSend,1);
            
            if (xmitRecStatus >= 0) {  
                cr_val = 0x1;
                cr_val |= 0x2; 
                cr_val |= 0x4; 
                cr_val |= (toSend[0] << 8); 
                int Bstatus = USLOSS_DeviceOutput(USLOSS_TERM_DEV, termNum, (void*)(long)cr_val);
                MboxCondSend(writeCompleted[termNum], NULL, 0);
            } 
        }
    }
}

void diskd(char* arg) {
    int diskNum =  atoi(arg);
    while (1 == 1) {
        int status;
        waitDevice(USLOSS_DISK_DEV, diskNum, &status);
        MboxCondSend(diskTracks[diskNum], NULL, 0);

        /* If there is an item waiting in the queue, remove it */
        // if (diskQueue != NULL) {
        //     MboxCondSend(diskQueue->mboxId, NULL, 0);
        // }
    }
}

/*
 * Function:  phase4_init
 * ----------------------
 * This function initializes sleep items and resets buffers for terminal handling.
 * 
 * arguments:
 *  None
 * 
 * returns:
 *  void
 */
void phase4_init(void){
    for (int i = 0; i < MAXPROC; i++) {
        sleepItems[i].mboxId = 0;
        sleepItems[i].wakeupTime = 0;
        sleepItems[i].next = NULL;

        disks[i].next = NULL;
        disks[i].mboxId = 0;
        disks[i].first = 0;
    }

    for (int i = 0; i < 4; i++) {
        resetBuffer(i);
    }
}

/*
 * Function:  phase4_start_service_processes
 * ----------------------------------------
 * This function initializes various mailboxes and system call vectors required
 * for starting service processes related to terminal handling and clock.
 * It also initializes terminal control registers and creates processes for
 * terminal handling.
 * 
 * arguments:
 *  None
 * 
 * returns:
 *  void
 */
void phase4_start_service_processes(void){
    termMut[0] = MboxCreate(1, 0);
    termMut[1] = MboxCreate(1, 0);
    termMut[2] = MboxCreate(1, 0);
    termMut[3] = MboxCreate(1, 0);

    termSender[0] = MboxCreate(1, 1);
    termSender[1] = MboxCreate(1, 1);
    termSender[2] = MboxCreate(1, 1);
    termSender[3] = MboxCreate(1, 1);

    writeCompleted[0] = MboxCreate(0, 0);
    writeCompleted[1] = MboxCreate(0, 0);
    writeCompleted[2] = MboxCreate(0, 0);
    writeCompleted[3] = MboxCreate(0, 0);

    diskTracks[0] = MboxCreate(0, 0);
    diskTracks[1] = MboxCreate(0, 0);

    termRecvMbox[0] = MboxCreate(10, MAXLINE+1);
    termRecvMbox[1] = MboxCreate(10, MAXLINE+1);
    termRecvMbox[2] = MboxCreate(10, MAXLINE+1);
    termRecvMbox[3] = MboxCreate(10, MAXLINE+1);

    systemCallVec[SYS_SLEEP] = sleep;
    systemCallVec[SYS_TERMREAD] = termRead;
    systemCallVec[SYS_TERMWRITE] = termWrite;
    systemCallVec[SYS_DISKSIZE] = diskSize;
    systemCallVec[SYS_DISKREAD] = diskRead;
    systemCallVec[SYS_DISKWRITE] = diskWrite;

    spork("ClockD", clock, NULL, USLOSS_MIN_STACK, 1);

    spork("Term1D", termd, "0", USLOSS_MIN_STACK, 1);
    spork("Term2D", termd, "1", USLOSS_MIN_STACK, 1);
    spork("Term3D", termd, "2", USLOSS_MIN_STACK, 1);
    spork("Term4D", termd, "3", USLOSS_MIN_STACK, 1);

    spork("Disk1D", diskd, "0", USLOSS_MIN_STACK, 1);
    spork("Disk2D", diskd, "1", USLOSS_MIN_STACK, 1);


    int ctrReg = 0x6;
    USLOSS_DeviceOutput(USLOSS_TERM_DEV, 0, ctrReg);
    USLOSS_DeviceOutput(USLOSS_TERM_DEV, 1, ctrReg);
    USLOSS_DeviceOutput(USLOSS_TERM_DEV, 2, ctrReg);
    USLOSS_DeviceOutput(USLOSS_TERM_DEV, 3, ctrReg); 
}

/*
 * Function:  resetBuffer
 * --------------------
 * This function resets the contents of a specific buffer to null characters ('\0').
 * It iterates through the buffer and assigns '\0' to each element.
 * 
 * arguments:
 *  int bufferToReset - the index of the buffer to be reset
 * 
 * returns:
 *  void
 */
void resetBuffer(int bufferToReset) {
    for (int i = 0; i < MAXLINE + 1; i++) {
            buffers[bufferToReset][i] = '\0';
    }
}

void appendQueue(struct diskInfo* disk) {
    /* If queue is empty then add disk */
    if ( diskQueue == NULL ) {
        diskQueue = disk;
        return;
    }

    /* If this one should go in the front */
    struct diskInfo* cur = diskQueue;
    if (disk->first < cur->first) {
        disk->next = diskQueue;
        diskQueue = disk;
        return;
    }

    /* Otherwise find the desired location */
    struct diskInfo* prev = cur;
    cur = cur->next;
    while (cur != NULL) {
        if (disk->first > cur->first) {
            prev->next = disk;
            disk->next = cur;
            return;
        }
        prev = cur;
        cur = cur->next;
    }

    /* If no location found, put it at the end */
    prev->next = disk;
}

void seek(int track, int unit) {
    USLOSS_DeviceRequest req;

    req.opr = USLOSS_DISK_TRACKS;
    req.reg1 = track;

    USLOSS_DeviceOutput(USLOSS_DISK_DEV, unit, &req);
    MboxRecv(diskTracks[unit], NULL, 0);  
}