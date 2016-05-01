#include "config.h"
#include "event.h"

#include <fcntl.h>
#include <string.h>
#include <sys/errno.h>
#include <unistd.h>

typedef struct EventLoop EventLoop;

typedef void (*FinalizeProc)(EventLoop* loop);

struct EventLoop {
    Tcl_Condition cond;
    int control[2];
    FinalizeProc finalize;
    Tcl_ThreadId id;
    Tcl_Mutex mutex;
    bool started;
};

static EventLoop* globalLoop = NULL;
TCL_DECLARE_MUTEX(globalMutex);

static Tcl_ThreadCreateType Loop(ClientData clientData) {
    EventLoop* loop = clientData;

    Tcl_MutexLock(&loop->mutex);
    loop->started = true;
    Tcl_ConditionNotify(&loop->cond);
    Tcl_MutexUnlock(&loop->mutex);

    loop->finalize(loop);
    ckfree(loop);

    TCL_THREAD_CREATE_RETURN;
}

static EventLoop* Start(Tcl_Interp* interp, FinalizeProc finalize) {
    EventLoop* loop = ckalloc(sizeof(EventLoop));
    int result = TCL_ERROR;

    loop->cond = NULL;
    loop->finalize = finalize;
    loop->id = NULL;
    loop->mutex = NULL;
    loop->started = false;

    if (pipe2(loop->control, O_CLOEXEC | O_NONBLOCK)) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(strerror(errno), -1));
        result = TCL_ERROR;
    }
    else {
        Tcl_MutexLock(&loop->mutex);
        result = Tcl_CreateThread(&loop->id, Loop, loop,
                                  TCL_THREAD_STACK_DEFAULT, TCL_THREAD_NOFLAGS);
        if (result == TCL_OK) {
            while (!loop->started)
                Tcl_ConditionWait(&loop->cond, &loop->mutex, NULL);
        }
        Tcl_MutexUnlock(&loop->mutex);
    }

    if (result != TCL_OK) {
        ckfree(loop);
        loop = NULL;
    }

    return loop;
}

static void Finalize(EventLoop* loop) {
    Tcl_MutexLock(&globalMutex);
    if (loop == globalLoop)
        globalLoop = NULL;
    Tcl_MutexUnlock(&globalMutex);
}

bool SshEventInit(Tcl_Interp* interp) {
    if (!globalLoop) {
        Tcl_MutexLock(&globalMutex);
        if (!globalLoop)
            globalLoop = Start(interp, Finalize);
        Tcl_MutexUnlock(&globalMutex);
    }

    return (globalLoop != NULL);
}
