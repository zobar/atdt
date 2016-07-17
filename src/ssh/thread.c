#include "sshInt.h"

#include <libssh/callbacks.h>

static int MutexDestroy(void** lock) {
    Tcl_Mutex* mutex = *lock;

    Tcl_MutexFinalize(mutex);

    return 0;
}

static int MutexInit(void** lock) {
    Tcl_Mutex* mutex = ckalloc(sizeof(Tcl_Mutex));

    *mutex = NULL;
    *lock = mutex;

    return 0;
}

static int MutexLock(void** lock) {
    Tcl_Mutex* mutex = *lock;

    Tcl_MutexLock(mutex);

    return 0;
}

static int MutexUnlock(void** lock) {
    Tcl_Mutex* mutex = *lock;

    Tcl_MutexUnlock(mutex);

    return 0;
}

static unsigned long ThreadId() {
    return (unsigned long) Tcl_GetCurrentThread();
}

bool SshInitThread(unused Tcl_Interp* interp) {
    static bool init = true;
    static struct ssh_threads_callbacks_struct callbacks = {
        .mutex_destroy = MutexDestroy,
        .mutex_init    = MutexInit,
        .mutex_lock    = MutexLock,
        .mutex_unlock  = MutexUnlock,
        .thread_id     = ThreadId,
        .type          = "tcl"
    };
    TCL_DECLARE_MUTEX(mutex);

    if (init) {
        Tcl_MutexLock(&mutex);
        if (init) {
            ssh_threads_set_callbacks(&callbacks);
            ssh_init();
            init = false;
        }
        Tcl_MutexUnlock(&mutex);
    }

    return true;
}
