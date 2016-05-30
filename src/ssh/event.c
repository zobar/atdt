#include "config.h"
#include "event.h"

#include "error.h"
#include "ssh.h"

#include "nonblocking.h"
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

typedef struct {
    ssh_event    event;
    Tcl_ThreadId threadId;
    int          readFd;
} EventLoop;

typedef void (*MessageCallback)(EventLoop* loop, ClientData clientData);

typedef struct {
    MessageCallback callback;
    ClientData      clientData;
} Message;

typedef struct {
    ClientData         clientData;
    ssh_event_callback callback;
    short              events;
    socket_t           fd;
} FdEventHandler;

static int writeFd = -1;

TCL_DECLARE_MUTEX(mutex);

static int Receive(unused socket_t fd, unused int revents,
                   unused ClientData clientData) {
    return SSH_OK;
}

static Tcl_ThreadCreateType Loop(ClientData clientData) {
    EventLoop* loop = clientData;
    int status = SSH_OK;

    status = ssh_event_add_fd(loop->event, loop->readFd, POLLIN, Receive, loop);

    while (status == SSH_OK) {
        status = ssh_event_dopoll(loop->event, -1);

        if (status == SSH_OK) {
            errno = 0;
            while(errno == 0) {
                Message message;

                if (read(loop->readFd, &message, sizeof(Message))
                        == sizeof(Message))
                    message.callback(loop, message.clientData);
                else if (errno != EAGAIN)
                    status = SSH_ERROR;
            }
        }
    }

    ssh_event_remove_fd(loop->event, loop->readFd);
    ssh_event_free(loop->event);
    close(loop->readFd);
    ckfree(loop);

    TCL_THREAD_CREATE_RETURN;
}

static int Start(Tcl_Interp* interp) {
    int control[2];
    int fd = -1;
    EventLoop* loop = NULL;

    if (pipe2(control, O_CLOEXEC | O_NONBLOCK) == 0) {
        int status = 0;

        loop = ckalloc(sizeof(EventLoop));
        loop->event  = ssh_event_new();
        loop->readFd = control[0];
        status = Tcl_CreateThread(&loop->threadId, Loop, loop,
                                  TCL_THREAD_STACK_DEFAULT, TCL_THREAD_NOFLAGS);

        if (status == TCL_OK)
            fd = control[1];
        else {
            ssh_event_free(loop->event);
            close(control[0]);
            close(control[1]);
            ckfree(loop);
            loop = NULL;
        }
    }
    else
        PosixError(interp, errno);

    return fd;
}

static int Send(Tcl_Interp* interp, Message* message) {
    int result = TCL_ERROR;

    Tcl_MutexLock(&mutex);

    if (writeFd == -1)
        writeFd = Start(interp);

    if (writeFd != -1) {
        if (write(writeFd, message, sizeof(Message)) == sizeof(Message))
            result = TCL_OK;
        else {
            PosixError(interp, errno);

            if (errno != EAGAIN) {
                close(writeFd);
                writeFd = -1;
            }
        }
    }

    Tcl_MutexUnlock(&mutex);

    return result;
}

static void AddFdEventHandler(EventLoop* loop, ClientData clientData) {
    FdEventHandler* handler = clientData;

    ssh_event_add_fd(loop->event, handler->fd, handler->events,
                     handler->callback, handler->clientData);

    ckfree(handler);
}

int SshAddFdEventHandler(Tcl_Interp* interp, socket_t fd, short events,
                         ssh_event_callback callback, ClientData clientData) {
    FdEventHandler* handler = ckalloc(sizeof(FdEventHandler));
    Message message = {
        .callback   = AddFdEventHandler,
        .clientData = handler
    };
    int result = TCL_ERROR;

    handler->callback   = callback;
    handler->clientData = clientData;
    handler->events     = events;
    handler->fd         = fd;

    result = Send(interp, &message);

    if (result != TCL_OK)
        ckfree(handler);

    return result;
}

static void RemoveFdEventHandler(EventLoop* loop, ClientData clientData) {
    socket_t* fdPtr = clientData;

    ssh_event_remove_fd(loop->event, *fdPtr);

    ckfree(fdPtr);
}

int SshRemoveFdEventHandler(Tcl_Interp* interp, socket_t fd) {
    socket_t* fdPtr = ckalloc(sizeof(socket_t));
    Message message = {
        .callback   = RemoveFdEventHandler,
        .clientData = fdPtr
    };
    int result = TCL_ERROR;

    *fdPtr = fd;
    result = Send(interp, &message);

    if (result != TCL_OK)
        ckfree(fdPtr);

    return TCL_OK;
}
