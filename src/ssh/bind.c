#include "bind.h"
#include "metadata.h"

static int Accept(unused ClientData clientData, Tcl_Interp* interp,
                  Tcl_ObjectContext objectContext, int objc,
                  Tcl_Obj* const* objv) {
    ssh_bind bind = SshGetBind(interp, Tcl_ObjectContextObject(objectContext));
    int result = TCL_ERROR;
    int skip = Tcl_ObjectContextSkippedArgs(objectContext);

    if (objc == skip + 2) {
        ssh_session session = SshGetSessionObj(interp, objv[skip]);
        if (session) {
            Tcl_Channel channel =
                    Tcl_GetChannel(interp, Tcl_GetString(objv[skip + 1]), NULL);

            if (channel != NULL) {
                intptr_t in = 0;
                intptr_t out = 0;

                if (Tcl_GetChannelHandle(channel, TCL_READABLE,
                                         (void*) &in) == TCL_OK
                        && Tcl_GetChannelHandle(channel, TCL_WRITABLE,
                                                (void*) &out) == TCL_OK
                        && in == out) {
                    int status = ssh_bind_accept_fd(bind, session, in);

                    if (status == SSH_OK)
                        result = TCL_OK;
                    else {
                        Tcl_SetObjResult(interp,
                                         Tcl_NewStringObj(ssh_get_error(bind),
                                                          -1));
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
    else
        Tcl_WrongNumArgs(interp, skip, objv, "session channel");

    return result;
}

static int Configure(unused ClientData clientData, Tcl_Interp* interp,
                     Tcl_ObjectContext objectContext, int objc,
                     Tcl_Obj* const* objv) {
    enum options {BLOCKING, RSA_KEY};
    static const char* keys[] = {"-blocking", "-rsakey", NULL};

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
                    {
                        int blocking = -1;

                        result = Tcl_GetBooleanFromObj(interp, arg, &blocking);
                        if (result != TCL_ERROR) {
                            printf("-blocking %i\n", blocking);
                            ssh_bind_set_blocking(bind, blocking);
                        }
                    }
                    break;

                case RSA_KEY:
                    {
                        char* rsaKey = Tcl_GetString(arg);

                        printf("-rsakey %s\n", rsaKey);
                        if (ssh_bind_options_set(bind, SSH_BIND_OPTIONS_RSAKEY,
                                                 rsaKey) != SSH_OK) {
                            Tcl_SetObjResult(
                                    interp,
                                    Tcl_NewStringObj(ssh_get_error(bind), -1));
                            result = TCL_ERROR;
                        }
                    }
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

static int Constructor(ClientData clientData, Tcl_Interp* interp,
                       Tcl_ObjectContext objectContext, int objc,
                       Tcl_Obj* const* objv) {
    SshSetBind(Tcl_ObjectContextObject(objectContext), ssh_bind_new());

    return Configure(clientData, interp, objectContext, objc, objv);
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
