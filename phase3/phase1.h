/*
 * These are the definitions for phase1 of the project (the kernel).
 */

#ifndef _PHASE1_H
#define _PHASE1_H

#include <usloss.h>

/*
 * Maximum number of processes. 
 */

#define MAXPROC      50

/*
 * Maximum length of a process name
 */

#define MAXNAME      50

/*
 * Maximum length of string argument passed to a newly created process
 */

#define MAXARG       100

/*
 * Maximum number of syscalls.
 */

#define MAXSYSCALLS  50


/* 
 * These functions must be provided by Phase 1.
 */

extern void phase1_init(void);
extern int  spork(char *name, int(*func)(char *), char *arg,
                  int stacksize, int priority);
extern int  join(int *status);

/* this has two versions.  The "phase_1a" version exists because, in 1A,
 * we don't have the dispatcher working yet.  The other version is the
 * one that you should use for everything *except* Phase 1a.
 */
extern void quit_phase_1a(int status, int switchToPid) __attribute__((__noreturn__));
extern void quit         (int status)                  __attribute__((__noreturn__));

extern int  getpid(void);
extern void dumpProcesses(void);
extern void blockMe(int block_status);
extern int  unblockProc(int pid);

extern void dispatcher(void);

extern int currentTime(void);



/* this is the main function for the init process.  The student code
 * must create a process (PID 1) use this as the main() function.
 */
extern int init_main(char*);



/* these functions are also called by the phase 1 code, from inside
 * init_main().  They are called first; after they return, init()
 * enters an infinite loop, just join()ing with children forever.
 *
 * In early phases, these are provided (as NOPs) by the testcase.
 */
extern void phase2_start_service_processes(void);
extern void phase3_start_service_processes(void);
extern void phase4_start_service_processes(void);
extern void phase5_start_service_processes(void);

/* this function is called by the init process, after the service
 * processes are running, to start whatever processes the testcase
 * wants to run.  This may call spork() or spawn() many times, and
 * block as long as you want.  When it returns, Halt() will be
 * called by the Phase 1 code (nonzero means error).
 */
extern int testcase_main(void);



#endif /* _PHASE1_H */
