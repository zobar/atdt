#include "sshInt.h"

static int Configure(
        unused ClientData clientData, Tcl_Interp* interp,
        Tcl_ObjectContext objectContext, int objc, Tcl_Obj* const* objv) {
    enum options {BLOCKING};
    static const char* keys[] = {"-blocking", NULL};

    int i = 0;
    int result = TCL_OK;
    ssh_session session = SshGetSession(
            interp, Tcl_ObjectContextObject(objectContext));
    int skip = Tcl_ObjectContextSkippedArgs(objectContext);

    for (i = skip; i < objc && result == TCL_OK; ++i) {
        int option = 0;

        result = Tcl_GetIndexFromObj(
                interp, objv[i], keys, "option", 0, &option);
        if (result == TCL_OK) {
            if (++i < objc) {
                Tcl_Obj* arg = objv[i];

                switch (option) {
                case BLOCKING:
                    {
                        int blocking = -1;
                        result = Tcl_GetBooleanFromObj(interp, arg, &blocking);
                        if (result != TCL_ERROR)
                            ssh_set_blocking(session, blocking);
                    }
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

static int Constructor(
        ClientData clientData, Tcl_Interp* interp,
        Tcl_ObjectContext objectContext, int objc, Tcl_Obj* const* objv) {
    SshSetSession(Tcl_ObjectContextObject(objectContext), ssh_new());

    return Configure(clientData, interp, objectContext, objc, objv);
}

static int HandleKeyExchange(
        unused ClientData clientData, Tcl_Interp* interp,
        Tcl_ObjectContext objectContext, int objc, Tcl_Obj* const* objv) {
    int result = TCL_ERROR;
    int skip = Tcl_ObjectContextSkippedArgs(objectContext);

    if (skip == objc) {
        Tcl_Object self = Tcl_ObjectContextObject(objectContext);
        ssh_session session = SshGetSession(interp, self);

        if (session != NULL) {
            int status = ssh_handle_key_exchange(session);

            if (status == SSH_OK)
                result = TCL_OK;
            else {
                Tcl_SetObjResult(
                        interp, Tcl_NewStringObj(ssh_get_error(session), -1));
            }
        }
    }
    else
        Tcl_WrongNumArgs(interp, skip, objv, NULL);

    return result;
}

bool SshSessionInit(Tcl_Interp* interp) {
    static const Tcl_MethodType constructor = {
        .version  = TCL_OO_METHOD_VERSION_CURRENT,
        .name     = "Constructor",
        .callProc = Constructor
    };
    static const Tcl_MethodType configure = {
        .version  = TCL_OO_METHOD_VERSION_CURRENT,
        .name     = "configure",
        .callProc = Configure
    };
    static const Tcl_MethodType handleKeyExchange = {
        .version  = TCL_OO_METHOD_VERSION_CURRENT,
        .name     = "handleKeyExchange",
        .callProc = HandleKeyExchange
    };
    static const Tcl_MethodType* methods[] =
            {&configure, &handleKeyExchange, NULL};
    Tcl_Class class = SshNewClass(
            interp, "::ssh::session", &constructor, NULL, methods);

    return (class != NULL);
}

Tcl_Object SshNewSession(Tcl_Interp* interp, ssh_session session) {
    Tcl_Object object = SshNewInstance(interp, "::ssh::session", NULL);

    SshSetSession(object, session);

    return object;
}
