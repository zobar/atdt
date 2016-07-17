#include "sshInt.h"

static int Accept(
        unused ClientData clientData, Tcl_Interp* interp,
        Tcl_ObjectContext objectContext, int objc, Tcl_Obj* const* objv) {
    int result = TCL_ERROR;
    int skip = Tcl_ObjectContextSkippedArgs(objectContext);

    if (skip == objc) {
        Tcl_Object object = Tcl_ObjectContextObject(objectContext);
        ssh_message message = SshGetMessage(interp, object);
        ssh_session session = SshGetSessionRef(interp, object);

        if (message != NULL) {
            result = SshLibError(
                    interp, session,
                    ssh_message_auth_reply_success(message, false));
            SshDestroyMessage(interp, object);
        }
    }
    else
        Tcl_WrongNumArgs(interp, skip, objv, NULL);

    return result;
}

static int Reject(
        unused ClientData clientData, Tcl_Interp* interp,
        Tcl_ObjectContext objectContext, int objc, Tcl_Obj* const* objv) {
    int result = TCL_ERROR;
    int skip = Tcl_ObjectContextSkippedArgs(objectContext);

    if (skip == objc) {
        Tcl_Object object = Tcl_ObjectContextObject(objectContext);
        ssh_message message = SshGetMessage(interp, object);
        ssh_session session = SshGetSessionRef(interp, object);

        if (message != NULL) {
            result = SshLibError(
                    interp, session, ssh_message_reply_default(message));
            SshDestroyMessage(interp, object);
        }
    }
    else
        Tcl_WrongNumArgs(interp, skip, objv, NULL);

    return result;
}

static int User(unused ClientData clientData, Tcl_Interp* interp,
        Tcl_ObjectContext objectContext, int objc, Tcl_Obj* const* objv) {
    int result = TCL_ERROR;
    int skip = Tcl_ObjectContextSkippedArgs(objectContext);

    if (skip == objc) {
        Tcl_Object object = Tcl_ObjectContextObject(objectContext);
        ssh_message message = SshGetMessage(interp, object);

        if (message != NULL) {
            const char* user = ssh_message_auth_user(message);
            if (user != NULL) {
                Tcl_SetObjResult(interp, Tcl_NewStringObj(user, -1));
                result = TCL_OK;
            }
        }
    }
    else
        Tcl_WrongNumArgs(interp, skip, objv, NULL);

    return result;
}

void SshDestroyMessage(Tcl_Interp* interp, Tcl_Object object) {
    SshDestroyInstance(interp, object);
}

bool SshInitMessage(Tcl_Interp* interp) {
    static const Tcl_MethodType accept = {
        .callProc = Accept,
        .name     = "accept",
        .version  = TCL_OO_METHOD_VERSION_CURRENT
    };
    static const Tcl_MethodType reject = {
        .callProc = Reject,
        .name     = "reject",
        .version  = TCL_OO_METHOD_VERSION_CURRENT
    };
    static const Tcl_MethodType user = {
        .callProc = User,
        .name     = "user",
        .version  = TCL_OO_METHOD_VERSION_CURRENT
    };
    static const Tcl_MethodType* methods[] =
            {&accept, &reject, &user, NULL};
    Tcl_Class class = SshNewClass(interp, "::ssh::message", NULL, NULL, methods);

    return (class != NULL);
}

Tcl_Object SshNewMessage(Tcl_Interp* interp) {
    return SshNewInstance(interp, "::ssh::message", NULL);
}
