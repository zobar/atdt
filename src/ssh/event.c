#include "config.h"
#include "event.h"

#include "error.h"
#include "ssh.h"

#include "nonblocking.h"
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

typedef void (*Action)(Tcl_Interp* interp, ClientData clientData);

typedef struct EventLoop {
    ssh_event    event;
    Tcl_ThreadId id;
    int          read;
    int          write;
} EventLoop;

static EventLoop* currentLoop = NULL;
TCL_DECLARE_MUTEX(mutex);

static int Control(socket_t fd, unused int revents,
                   unused ClientData clientData) {
    Tcl_MutexLock(&mutex);
    while (currentLoop && errno == 0) {
        Tcl_Condition* cond = NULL;
        ssize_t bytesRead = read(fd, &cond, sizeof(cond));

        if (bytesRead == sizeof(cond)) {
            Tcl_ConditionNotify(cond);
            Tcl_ConditionWait(cond, &mutex, NULL);
        }
        else if (errno != EAGAIN)
            currentLoop = NULL;
    }
    Tcl_MutexUnlock(&mutex);

    return SSH_OK;
}

static Tcl_ThreadCreateType Loop(ClientData clientData) {
    EventLoop* loop = clientData;
    int status = ssh_event_add_fd(loop->event, loop->read, POLLIN, Control,
                                  loop);

    while (status == SSH_OK && loop == currentLoop)
        status = ssh_event_dopoll(loop->event, -1);

    ssh_event_free(loop->event);
    close(loop->read);
    ckfree(loop);

    TCL_THREAD_CREATE_RETURN;
}

static int Start(Tcl_Interp* interp) {
    int control[2];
    int result = PosixError(interp, pipe2(control, O_CLOEXEC | O_NONBLOCK));

    if (result == TCL_OK) {
        currentLoop = ckalloc(sizeof(EventLoop));
        currentLoop->event = ssh_event_new();
        currentLoop->read = control[0];
        currentLoop->write = control[1];
        result = Tcl_CreateThread(&currentLoop->id, Loop, currentLoop,
                                  TCL_THREAD_STACK_DEFAULT, TCL_THREAD_NOFLAGS);
    }
    return result;
}

static int Send(Tcl_Interp* interp, Action action, ClientData clientData) {
    int result = TCL_OK;

    Tcl_MutexLock(&mutex);

    if (!currentLoop)
        result = Start(interp);

    if (result == TCL_OK) {
        Tcl_Condition cond = NULL;
        Tcl_Condition* condPtr = &cond;

        if (write(currentLoop->write, &condPtr, sizeof(condPtr)) == -1)
            result = PosixError(interp, errno);
        else {
            Tcl_ConditionWait(condPtr, &mutex, NULL);
            action(interp, clientData);
            Tcl_ConditionNotify(condPtr);
        }

        Tcl_ConditionFinalize(condPtr);
    }

    Tcl_MutexUnlock(&mutex);

    return result;
}

typedef struct AddFdEventRequest {
    ssh_event_callback cb;
    short              events;
    socket_t           fd;
    int                result;
    ClientData         clientData;
} AddFdEventRequest;

static void AddFdEvent(unused Tcl_Interp* interp, ClientData clientData) {
    AddFdEventRequest* request = clientData;

    if (ssh_event_add_fd(currentLoop->event, request->fd, request->events,
                         request->cb, request->clientData) != SSH_OK) {
        Tcl_SetObjResult(interp,
                         Tcl_NewStringObj("Cannot add file descriptor event",
                                          -1));
        request->result = TCL_ERROR;
    }
}

int SshAddFdEvent(Tcl_Interp* interp, socket_t fd, short events,
                  ssh_event_callback cb, ClientData clientData) {
    AddFdEventRequest request = {
        .cb         = cb,
        .events     = events,
        .fd         = fd,
        .result     = TCL_OK,
        .clientData = clientData
    };
    int result = Send(interp, AddFdEvent, &request);

    if (result == TCL_OK)
        result = request.result;

    return result;
}
