
#include <memory.h>
#include <assert.h>

#include <usloss.h>
#include <usyscall.h>

#include "phase3_usermode.h"

#define TODO() do { USLOSS_Console("TODO() at %s:%d\n", __func__,__LINE__); *(char*)7 = 0; } while(0)



/* helper function */
static void require_user_mode(const char *func)
{
    if (USLOSS_PsrGet() != USLOSS_PSR_CURRENT_INT)
    {
        USLOSS_Console("ERROR: Someone attempted to call %s while in kernel mode!\n", func);
        USLOSS_Halt(1);
    }
}



int Spawn(char *name, int (*func)(char*), char *arg, int stack_size,
          int priority, int *pid)
{
    require_user_mode(__func__);

    USLOSS_Sysargs args;
    memset(&args, 0, sizeof(args));

    args.number = SYS_SPAWN;
    args.arg1   = func;
    args.arg2   = arg;
    args.arg3   = (void*)(long)stack_size;
    args.arg4   = (void*)(long)priority;
    args.arg5   = name;
    USLOSS_Syscall(&args);

    *pid = (int)(long)args.arg1;
    return (int)(long)args.arg4;
}



int Wait(int *pid, int *status)
{
    require_user_mode(__func__);

    USLOSS_Sysargs args;
    memset(&args, 0, sizeof(args));

    args.number = SYS_WAIT;
    USLOSS_Syscall(&args);

    *status = (int)(long)args.arg2;
    *pid    = (int)(long)args.arg1;
    return    (int)(long)args.arg4;
}



void Terminate(int status)
{
    require_user_mode(__func__);

    USLOSS_Sysargs args;
    memset(&args, 0, sizeof(args));

    args.number = SYS_TERMINATE;
    args.arg1   = (void*)(long)status;
    USLOSS_Syscall(&args);

    // never returns!
    assert(0);
}



void GetTimeofDay(int *tod)
{
    require_user_mode(__func__);

    USLOSS_Sysargs args;
    memset(&args, 0, sizeof(args));

    args.number = SYS_GETTIMEOFDAY;
    USLOSS_Syscall(&args);

    *tod = (int)(long)args.arg1;
}



void CPUTime(int *cpu)
{
    require_user_mode(__func__);

    USLOSS_Sysargs args;
    memset(&args, 0, sizeof(args));

    args.number = SYS_GETPROCINFO;
    USLOSS_Syscall(&args);

    *cpu = (int)(long)args.arg1;
}



void GetPID(int *pid)
{
    require_user_mode(__func__);

    USLOSS_Sysargs args;
    memset(&args, 0, sizeof(args));

    args.number = SYS_GETPID;
    USLOSS_Syscall(&args);

    *pid = (int)(long)args.arg1;
}



int SemCreate(int value, int *semaphore)
{
    require_user_mode(__func__);

    USLOSS_Sysargs args;
    memset(&args, 0, sizeof(args));

    args.number = SYS_SEMCREATE;
    args.arg1 = (void*)(long)value;
    USLOSS_Syscall(&args);

    *semaphore = (int)(long)args.arg1;
    return       (int)(long)args.arg4;
}



int SemP(int semaphore)
{
    require_user_mode(__func__);

    USLOSS_Sysargs args;
    memset(&args, 0, sizeof(args));

    args.number = SYS_SEMP;
    args.arg1 = (void*)(long)semaphore;
    USLOSS_Syscall(&args);

    return (int)(long)args.arg4;
}



int SemV(int semaphore)
{
    require_user_mode(__func__);

    USLOSS_Sysargs args;
    memset(&args, 0, sizeof(args));

    args.number = SYS_SEMV;
    args.arg1 = (void*)(long)semaphore;
    USLOSS_Syscall(&args);

    return (int)(long)args.arg4;
}



int SemFree(int semaphore)
{
    require_user_mode(__func__);

    USLOSS_Sysargs args;
    memset(&args, 0, sizeof(args));

    args.number = SYS_SEMFREE;
    args.arg1 = (void*)(long)semaphore;
    USLOSS_Syscall(&args);

    return (int)(long)args.arg4;
}

