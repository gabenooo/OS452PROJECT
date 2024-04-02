#include <phase4.h>
#include <phase3.h>
#include <phase2.h>
#include <phase1.h>

struct sleepItem {
    int procId;
    int mboxId;
    int wakeupTime;
    struct sleepItem next;
};

struct sleepItem sleepQueue[MAXPROC];