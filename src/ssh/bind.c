#include "config.h"
#include "bind.h"

#include "error.h"
#include "event.h"
#include "metadata.h"
#include "oo.h"
#include "ssh.h"

#include <poll.h>

static int Accept(unused ClientData clientData, Tcl_Interp* interp,
                  Tcl_ObjectContext objectContext, int objc,
                  Tcl_Obj* const* objv) {
    ssh_bind bind = SshGetBind(interp, Tcl_ObjectContextObject(objectContext));
    int result = TCL_ERROR;
    int skip = Tcl_ObjectContextSkippedArgs(objectContext);

    if (objc == skip + 2) {
        Tcl_Object sessionObject = Tcl_GetObjectFromObj(interp, objv[skip]);

        if (sessionObject != NULL) {
            ssh_session session = SshGetSession(interp, sessionObject);

            if (session) {
                Tcl_Channel channel =
                        Tcl_GetChannel(interp, Tcl_GetString(objv[skip + 1]),
                                       NULL);

                if (channel != NULL) {
                    intptr_t in = 0;
                    intptr_t out = 0;

                    if (Tcl_GetChannelHandle(channel, TCL_READABLE,
                                             (void*) &in) == TCL_OK
                            && Tcl_GetChannelHandle(channel, TCL_WRITABLE,
                                                    (void*) &out) == TCL_OK
                            && in == out) {
                        int status = ssh_bind_accept_fd(bind, session, in);

                        if (status == SSH_OK) {
                            SshSetChannel(sessionObject, channel);
                            result = TCL_OK;
                        }
                        else {
                            Tcl_SetObjResult(
                                    interp,
                                    Tcl_NewStringObj(ssh_get_error(bind), -1));
                        }
                    }
                    else {
                        Tcl_SetObjResult(interp,
                                         Tcl_NewStringObj("channel must be a readable, writable socket",
                                                          -1));
                    }
                }
            }
        }
    }
    else
        Tcl_WrongNumArgs(interp, skip, objv, "session channel");

    return result;
}

static int SetBlocking(Tcl_Interp* interp, ssh_bind bind, Tcl_Obj* arg) {
    int blocking = -1;
    int result = Tcl_GetBooleanFromObj(interp, arg, &blocking);

    if (result != TCL_ERROR)
        ssh_bind_set_blocking(bind, blocking);

    return result;
}

static int SetMyPort(Tcl_Interp* interp, ssh_bind bind, Tcl_Obj* arg) {
    int port = 0;
    int result = Tcl_GetIntFromObj(interp, arg, &port);

    if (result != TCL_ERROR) {
        result = SshError(interp, bind,
                          ssh_bind_options_set(bind, SSH_BIND_OPTIONS_BINDPORT,
                                               &port));
    }

    return result;
}

static int SetRsaKey(Tcl_Interp* interp, ssh_bind bind, Tcl_Obj* arg) {
    char* rsaKey = Tcl_GetString(arg);

    return SshError(interp, bind,
                    ssh_bind_options_set(bind, SSH_BIND_OPTIONS_RSAKEY,
                                         rsaKey));
}

static int Configure(unused ClientData clientData, Tcl_Interp* interp,
                     Tcl_ObjectContext objectContext, int objc,
                     Tcl_Obj* const* objv) {
    enum options {BLOCKING, MY_PORT, RSA_KEY};
    static const char* keys[] = {"-blocking", "-myport", "-rsakey", NULL};

    ssh_bind bind = SshGetBind(interp, Tcl_ObjectContextObject(objectContext));
    int result = TCL_OK;
    int skip = Tcl_ObjectContextSkippedArgs(objectContext);
    int i = skip;

    for (i = skip; i < objc && result == TCL_OK; ++i) {
        int option = 0;

        result = Tcl_GetIndexFromObj(interp, objv[i], keys, "option", 0,
                                     &option);
        if (result == TCL_OK) {
            if (++i < objc) {
                Tcl_Obj* arg = objv[i];

                switch (option) {
                case BLOCKING:
                    result = SetBlocking(interp, bind, arg);
                    break;

                case MY_PORT:
                    result = SetMyPort(interp, bind, arg);
                    break;

                case RSA_KEY:
                    result = SetRsaKey(interp, bind, arg);
                    break;

                default:
                    result = TCL_ERROR;
                }
            }
            else {
                result = TCL_ERROR;
                Tcl_SetObjResult(interp,
                                 Tcl_ObjPrintf("\"%s\" option requires an additional argument",
                                               keys[option]));
            }
        }
    }

    return result;
}

static int Poop(socket_t fd, int revents, void* userdata) {
    printf("Poop! %i %i %p\n", fd, revents, userdata);
    return SSH_ERROR;
}

static int Constructor(ClientData clientData, Tcl_Interp* interp,
                       Tcl_ObjectContext objectContext, int objc,
                       Tcl_Obj* const* objv) {
    ssh_bind bind = ssh_bind_new();
    int result = TCL_OK;

    SshSetBind(Tcl_ObjectContextObject(objectContext), bind);
    result = Configure(clientData, interp, objectContext, objc, objv);
    if (result == TCL_OK) {
        result = SshError(interp, bind, ssh_bind_listen(bind));
        if (result == TCL_OK) {
            result = SshAddFdEvent(interp, ssh_bind_get_fd(bind),
                                   POLLIN | POLLOUT, Poop, NULL);
        }
    }

    return result;
}

bool SshBindInit(Tcl_Interp* interp) {
    static const Tcl_MethodType constructor = {
        .version  = TCL_OO_METHOD_VERSION_CURRENT,
        .name     = "Constructor",
        .callProc = Constructor
    };
    static const Tcl_MethodType accept = {
        .version  = TCL_OO_METHOD_VERSION_CURRENT,
        .name     = "accept",
        .callProc = Accept
    };
    static const Tcl_MethodType configure = {
        .version  = TCL_OO_METHOD_VERSION_CURRENT,
        .name     = "configure",
        .callProc = Configure
    };
    static const Tcl_MethodType* methods[] = {&accept, &configure, NULL};

    return SshNewClass(interp, "::ssh::bind", &constructor, methods) != NULL;
}
