#include "sshInt.h"

#include <poll.h>

typedef struct {
    Tcl_Event   event;
    Tcl_Object  object;
    ssh_session session;
} ConnectEvent;

static int SetConnect(
        unused Tcl_Interp* interp, Tcl_Object object, Tcl_Obj* arg) {
    SshSetConnect(object, arg);

    return TCL_OK;
}

static int SetPort(Tcl_Interp* interp, Tcl_Object object, Tcl_Obj* arg) {
    ssh_bind bind = SshGetBind(interp, object);
    int result = TCL_ERROR;

    if (bind != NULL) {
        int port = 0;

        result = Tcl_GetIntFromObj(interp, arg, &port);
        if (result != TCL_ERROR) {
            int status = ssh_bind_options_set(
                    bind, SSH_BIND_OPTIONS_BINDPORT, &port);

            result = SshLibError(interp, bind, status);
        }
    }

    return result;
}

static int SetRsaKey(Tcl_Interp* interp, Tcl_Object object, Tcl_Obj* arg) {
    ssh_bind bind = SshGetBind(interp, object);
    int result = TCL_ERROR;

    if (bind != NULL) {
        char* rsaKey = Tcl_GetString(arg);
        int status = ssh_bind_options_set(
                bind, SSH_BIND_OPTIONS_RSAKEY, rsaKey);

        result = SshLibError(interp, bind, status);
    }

    return result;
}

static int Configure(
        unused ClientData clientData, Tcl_Interp* interp,
        Tcl_ObjectContext objectContext, int objc, Tcl_Obj* const* objv) {
    enum options {CONNECT, PORT, RSA_KEY};
    static const char* keys[] = {"-connect", "-port", "-rsakey", NULL};

    Tcl_Object object = Tcl_ObjectContextObject(objectContext);
    int result = TCL_OK;
    int skip = Tcl_ObjectContextSkippedArgs(objectContext);
    int i = skip;

    for (i = skip; i < objc && result == TCL_OK; ++i) {
        int option = 0;

        result = Tcl_GetIndexFromObj(
                interp, objv[i], keys, "option", 0, &option);
        if (result == TCL_OK) {
            if (++i < objc) {
                Tcl_Obj* arg = objv[i];

                switch (option) {
                case CONNECT:
                    result = SetConnect(interp, object, arg);
                    break;

                case PORT:
                    result = SetPort(interp, object, arg);
                    break;

                case RSA_KEY:
                    result = SetRsaKey(interp, object, arg);
                    break;

                default:
                    result = TCL_ERROR;
                }
            }
            else {
                Tcl_Obj* message = Tcl_ObjPrintf(
                        "\"%s\" option requires an additional argument",
                        keys[option]);

                Tcl_SetObjResult(interp, message);
                result = TCL_ERROR;
            }
        }
    }

    return result;
}

static int ConnectTclCallback(Tcl_Event* evPtr, unused int flags) {
    ConnectEvent* event = (ConnectEvent*) evPtr;
    Tcl_Object object = event->object;
    Tcl_Obj* connect = SshGetConnect(NULL, object);
    Tcl_Interp* interp = SshGetInterp(NULL, object);

    if (interp != NULL && connect != NULL) {
        Tcl_Obj* callback = NULL;
        Tcl_Obj* callbackWords[] = {connect, NULL};
        int result = TCL_OK;
        Tcl_Object sessionObject = NULL;

        Tcl_Preserve(interp);

        sessionObject = SshNewSession(interp, event->session);
        callbackWords[1] = Tcl_GetObjectName(interp, sessionObject);
        callback = Tcl_ConcatObj(2, callbackWords);

        Tcl_IncrRefCount(callback);
        result = Tcl_EvalObjEx(
                interp, callback, TCL_EVAL_DIRECT | TCL_EVAL_GLOBAL);
        Tcl_DecrRefCount(callback);

        if (result != TCL_OK)
            Tcl_BackgroundException(interp, result);

        Tcl_Release(interp);
    }

    return 1;
}

static int ConnectSshCallback(
        unused socket_t fd, unused int revents, unused ClientData clientData) {
    Tcl_Object object = clientData;
    ssh_bind bind = SshGetBind(NULL, object);
    Tcl_ThreadId threadId = SshGetThreadId(NULL, object);

    if (bind != NULL && threadId != NULL) {
        ssh_session session = ssh_new();

        if (ssh_bind_accept(bind, session) == SSH_OK) {
            ConnectEvent* event = ckalloc(sizeof(ConnectEvent));

            event->event.proc = ConnectTclCallback;
            event->object     = object;
            event->session    = session;

            Tcl_ThreadQueueEvent(threadId, (Tcl_Event*) event, TCL_QUEUE_TAIL);
            Tcl_ThreadAlert(threadId);
        }
        else
            ssh_free(session);
    }

    return SSH_OK;
}

static int Constructor(
        ClientData clientData, Tcl_Interp* interp,
        Tcl_ObjectContext objectContext, int objc, Tcl_Obj* const* objv) {
    ssh_bind bind = ssh_bind_new();
    Tcl_Object object = Tcl_ObjectContextObject(objectContext);

    ssh_bind_set_blocking(bind, false);
    SshSetBind(object, bind);
    SshSetInterp(object, interp);
    SshSetThreadId(object, Tcl_GetCurrentThread());

    return Configure(clientData, interp, objectContext, objc, objv);
}

static int Destructor(
        unused ClientData clientData, unused Tcl_Interp* interp,
        unused Tcl_ObjectContext objectContext, unused int objc,
        unused Tcl_Obj* const* objv) {
    Tcl_Object object = Tcl_ObjectContextObject(objectContext);
    ssh_bind bind = SshGetBind(interp, object);

    if (bind != NULL) {
        int fd = ssh_bind_get_fd(bind);

        if (fd != -1)
            SshRemoveFdEventHandler(interp, fd);
    }

    return TCL_OK;
}

static int Listen(
        unused ClientData clientData, Tcl_Interp* interp,
        Tcl_ObjectContext objectContext, int objc, Tcl_Obj* const* objv) {
    int result = TCL_ERROR;
    int skip = Tcl_ObjectContextSkippedArgs(objectContext);

    if (skip == objc) {
        Tcl_Object object = Tcl_ObjectContextObject(objectContext);
        ssh_bind bind = SshGetBind(interp, object);

        if (bind != NULL) {
            result = SshLibError(interp, bind, ssh_bind_listen(bind));
            if (result == TCL_OK) {
                result = SshAddFdEventHandler(
                        interp, ssh_bind_get_fd(bind), POLLIN | POLLOUT,
                        ConnectSshCallback, object);
            }
        }
    }
    else
        Tcl_WrongNumArgs(interp, skip, objv, NULL);

    return result;
}

bool SshBindInit(Tcl_Interp* interp) {
    static const Tcl_MethodType configure = {
        .callProc = Configure,
        .name     = "configure",
        .version  = TCL_OO_METHOD_VERSION_CURRENT
    };
    static const Tcl_MethodType constructor = {
        .callProc = Constructor,
        .name     = "Constructor",
        .version  = TCL_OO_METHOD_VERSION_CURRENT
    };
    static const Tcl_MethodType destructor = {
        .callProc = Destructor,
        .name     = "Destructor",
        .version  = TCL_OO_METHOD_VERSION_CURRENT
    };
    static const Tcl_MethodType listen = {
        .callProc = Listen,
        .name     = "listen",
        .version  = TCL_OO_METHOD_VERSION_CURRENT
    };
    static const Tcl_MethodType* methods[] = {&configure, &listen, NULL};
    Tcl_Class class = SshNewClass(
            interp, "::ssh::bind", &constructor, &destructor, methods);

    return (class != NULL);
}
