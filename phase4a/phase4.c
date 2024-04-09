#include <phase3.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <phase4.h>
#include <phase2.h>
#include <phase1.h>

int CLOCK = 0;



int term0Mut = -1;
int term1Mut = -1;
int term2Mut = -1;
int term3Mut = -1;
int term0Sender = -1;
int term1Sender = -1;
int term2Sender = -1;
int term3Sender = -1;

int termRecvMbox[4];

char buffers[4][MAXLINE + 1];


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

    /* Error Checking */
    if (bufferSize <= 0 || unitID < 0 || unitID > 3) {
        args->arg4 = -1;
        return;
    }

    char fullBuffer[MAXLINE];
    memset(fullBuffer, '\0', MAXLINE);

    int status = MboxRecv(termRecvMbox[unitID], fullBuffer, MAXLINE+1);
    strncpy(buffer, fullBuffer, bufferSize);

    //USLOSS_Console("Status code is %d, recived on %d, Recieved string %s\n", status, termRecvMbox[unitID], fullBuffer);

    /* Set the out parameters */
    int actualLen = strlen(buffer);
    if (actualLen < bufferSize) {
        args->arg2 = actualLen;
    } else {
        args->arg2 = bufferSize;
    }
    args->arg4 = 0;

}

void termWrite(void* arg) {
    USLOSS_Sysargs *args = (USLOSS_Sysargs*) arg;
    USLOSS_Console("Write called\n");
    // grab lock 
    void * buffer = args->arg1;
    long bufferSize = args->arg2;
    long unitID = args->arg3;
    switch (unitID){
        case 0:  
            // gain lock
            MboxSend(term0Mut, NULL, NULL);
            for (int c = 0 ; c < bufferSize; c++){
                char* toSend = &buffer[c];
                MboxCondSend(term0Sender, toSend,1);
            }
            MboxRecv(term0Mut, NULL, NULL);

            break;
        case 1:
            MboxSend(term1Mut, NULL, NULL);
            for (int c = 0 ; c < bufferSize; c++){
                char* toSend = &buffer[c];
                MboxCondSend(term1Sender, toSend,1);
            }
            MboxRecv(term1Mut, NULL, NULL);
            break;
        case 2:
            MboxSend(term2Mut, NULL, NULL);
            for (int c = 0 ; c < bufferSize; c++){
                char* toSend = &buffer[c];
                MboxCondSend(term2Sender, toSend,1);
            }
            MboxRecv(term2Mut, NULL, NULL);
            break;
        case 3:
            MboxSend(term3Mut, NULL, NULL);
            for (int c = 0 ; c < bufferSize; c++){
                char* toSend = &buffer[c];
                MboxCondSend(term3Sender, toSend,1);
            }
            MboxRecv(term3Mut, NULL, NULL);
            break;
        default:
            break;

    }

    // *numCharsWritten = (long) sysArg.arg2;
    args->arg4 = 0;
    
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

        int recvStatus = USLOSS_TERM_STAT_RECV(status);
        int xmitStatus = USLOSS_TERM_STAT_XMIT(status);

        /* We are ready to recieve a character */
        if (recvStatus == 1) {
            char readChar = USLOSS_TERM_STAT_CHAR(status);
            size_t len = strlen(buffers[termNum]);
            buffers[termNum][len] = readChar;
            buffers[termNum][len + 1] = '\0';

            /* Checks if buffer is full or newline encounterd */
            if (len + 1 >= MAXLINE || readChar == '\n') {
                //USLOSS_Console("%s\n", buffers[termNum]);
                int mboxstatus = MboxCondSend(termRecvMbox[termNum], buffers[termNum], len+1);
                //USLOSS_Console("mbox status is %d on mbox %d, string is send is %s\n", mboxstatus, termRecvMbox[termNum], buffers[termNum]);

                resetBuffer(termNum);
            }

            
        }

        /* We are ready to send a character */
        if (xmitStatus == 1) {
            char toSend;
            int cr_val;
            switch (termNum){
                case 0: 
                    MboxRecv(term0Sender, toSend,1);
                    cr_val = 0x1; // this turns on the ’send char’ bit (USLOSS spec page 9)
                    cr_val |= 0x2; // recv int enable
                    cr_val |= 0x4; // xmit int enable
                    cr_val |= (toSend << 8); // the character to send
                    USLOSS_DeviceOutput(USLOSS_TERM_DEV, termNum, (void*)(long)cr_val);
                    break;
                case 1:
                    MboxRecv(term1Sender, toSend,1);
                    cr_val = 0x1; // this turns on the ’send char’ bit (USLOSS spec page 9)
                    cr_val |= 0x2; // recv int enable
                    cr_val |= 0x4; // xmit int enable
                    cr_val |= (toSend << 8); // the character to send
                    USLOSS_DeviceOutput(USLOSS_TERM_DEV, termNum, (void*)(long)cr_val);
                    break;
                case 2:
                    MboxRecv(term2Sender, toSend,1);
                    cr_val = 0x1; // this turns on the ’send char’ bit (USLOSS spec page 9)
                    cr_val |= 0x2; // recv int enable
                    cr_val |= 0x4; // xmit int enable
                    cr_val |= (toSend << 8); // the character to send
                    USLOSS_DeviceOutput(USLOSS_TERM_DEV, termNum, (void*)(long)cr_val);
                    break;
                case 3:
                    MboxRecv(term3Sender, toSend,1);
                    cr_val = 0x1; // this turns on the ’send char’ bit (USLOSS spec page 9)
                    cr_val |= 0x2; // recv int enable
                    cr_val |= 0x4; // xmit int enable
                    cr_val |= (toSend << 8); // the character to send
                    USLOSS_DeviceOutput(USLOSS_TERM_DEV, termNum, (void*)(long)cr_val);
                    break;
                default:
                    break;
            }
        }

        //USLOSS_Console("terminal interupt recv=%d xmit=%d\n", recvStatus,    term1Recv = MboxCreate(10, MAXLINE);

    }
}

void phase4_init(void){
    for (int i = 0; i < MAXPROC; i++) {
        sleepItems[i].mboxId = 0;
        sleepItems[i].wakeupTime = 0;
        sleepItems[i].next = NULL;
    }

    for (int i = 0; i < 4; i++) {
        resetBuffer(i);
    }
}

void phase4_start_service_processes(void){
    term0Mut = MboxCreate(0, 0);
    term1Mut = MboxCreate(0, 0);
    term2Mut = MboxCreate(0, 0);
    term3Mut = MboxCreate(0, 0);
    term0Sender = MboxCreate(1, 1);
    term1Sender = MboxCreate(1, 1);
    term2Sender = MboxCreate(1, 1);
    term3Sender = MboxCreate(1, 1);

    termRecvMbox[0] = MboxCreate(10, MAXLINE+1);
    termRecvMbox[1] = MboxCreate(10, MAXLINE+1);
    termRecvMbox[2] = MboxCreate(10, MAXLINE+1);
    termRecvMbox[3] = MboxCreate(10, MAXLINE+1);

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

void resetBuffer(int bufferToReset) {
    for (int i = 0; i < MAXLINE + 1; i++) {
            buffers[bufferToReset][i] = '\0';
    }
}
