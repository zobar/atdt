#include "sshInt.h"

static int Constructor(unused ClientData clientData, unused Tcl_Interp* interp,
        Tcl_ObjectContext objectContext, unused int objc,
        unused Tcl_Obj* const* objv) {
    SshSetSession(Tcl_ObjectContextObject(objectContext), ssh_new());

    return TCL_OK;
}

static void GetMessage(ClientData clientData, unused int mask) {
    Tcl_Object object = clientData;
    Tcl_Interp* interp = SshGetInterp(NULL, object);
    ssh_session session = SshGetSession(NULL, object);

    if (interp != NULL && session != NULL) {
        ssh_message message = ssh_message_get(session);
        int status = 0;

        Tcl_Preserve(interp);

        while (message != NULL) {
            Tcl_Obj* command[] = {NULL, NULL};
            int type = ssh_message_type(message);
            int subtype = ssh_message_subtype(message);

            switch (type) {
            case SSH_REQUEST_AUTH:
                switch (subtype) {
                case SSH_AUTH_METHOD_NONE:
                    command[0] = SshGetNoneAuthCallback(NULL, object);
                    break;

                default:
                    break;
                }
            default:
                break;
            }

            if (command[0] == NULL) {
                printf("No callback (%i/%i), rejecting\n", type, subtype);
                ssh_message_reply_default(message);
                ssh_message_free(message);
            }
            else {
                Tcl_Object messageObject = SshNewMessage(interp);

                printf("Found callback (%i/%i)\n", type, subtype);
                SshSetMessage(messageObject, message);
                SshSetSessionRef(messageObject, session);
                command[1] = Tcl_GetObjectName(interp, messageObject);
                SshCallBack(interp, 2, command);
            }

            message = ssh_message_get(session);
        }

        status = ssh_get_status(session);
        if (status != 0) {
            Tcl_Obj* command[] = {NULL};

            if ((status & SSH_CLOSED_ERROR) != 0) {
                command[0] = SshGetStatusClosedErrorCallback(NULL, object);
                SshCallBack(interp, 1, command);
            }
        }

        Tcl_Release(interp);
    }
}

static int HandleKeyExchange(
        unused ClientData clientData, Tcl_Interp* interp,
        Tcl_ObjectContext objectContext, int objc, Tcl_Obj* const* objv) {
    int result = TCL_ERROR;
    int skip = Tcl_ObjectContextSkippedArgs(objectContext);

    if (skip == objc) {
        Tcl_Object object = Tcl_ObjectContextObject(objectContext);
        Tcl_Channel channel = SshGetChannel(interp, object);
        ssh_session session = SshGetSession(interp, object);

        if (channel != NULL && session != NULL) {
            result = SshLibError(
                    interp, session, ssh_handle_key_exchange(session));
            if (result == TCL_OK) {
                ssh_set_blocking(session, false);
                result = Tcl_SetChannelOption(
                        interp, channel, "-blocking", "false");
                if (result == TCL_OK) {
                    GetMessage(object, TCL_READABLE);
                    Tcl_CreateChannelHandler(
                            channel, TCL_READABLE, GetMessage, object);
                }
            }
        }
    }
    else
        Tcl_WrongNumArgs(interp, skip, objv, NULL);

    return result;
}

static int SetCallback(
        unused ClientData clientData, unused Tcl_Interp* interp,
        unused Tcl_ObjectContext objectContext, unused int objc,
        unused Tcl_Obj* const* objv) {
    enum types {AUTH, STATUS};
    enum authSubtypes {NONE};
    enum statusSubtypes {CLOSED_ERROR};
    static const char* types[] = {"auth", "status", NULL};
    static const char* authSubtypes[] = {"none"};
    static const char* statusSubtypes[] = {"closedError"};

    Tcl_Object object = Tcl_ObjectContextObject(objectContext);
    int result = TCL_ERROR;
    int skip = Tcl_ObjectContextSkippedArgs(objectContext);

    if (objc == skip + 3) {
        Tcl_Obj* callback = objv[objc - 1];
        int type = -1;
        int subtype = -1;

        result = Tcl_GetIndexFromObj(
            interp, objv[skip], types, "type", 0, &type);
        if (result == TCL_OK) {
            switch (type) {
            case AUTH:
                result = Tcl_GetIndexFromObj(
                        interp, objv[skip + 1], authSubtypes, "event subtype",
                        0, &subtype);
                if (result == TCL_OK) {
                    switch (subtype) {
                    case NONE:
                        SshSetNoneAuthCallback(object, callback);
                        break;

                    default:
                        result = TCL_ERROR;
                    }
                }
                break;

            case STATUS:
                result = Tcl_GetIndexFromObj(
                        interp, objv[skip + 1], statusSubtypes,
                        "status subtype", 0, &subtype);
                if (result == TCL_OK) {
                    switch (subtype) {
                    case CLOSED_ERROR:
                        SshSetStatusClosedErrorCallback(object, callback);
                        break;

                    default:
                        result = TCL_ERROR;
                    }
                }
                break;

            default:
                result = TCL_ERROR;
            }
        }
    }
    else
        Tcl_WrongNumArgs(interp, skip, objv, "event callback");

    return result;
}

void SshDestroySession(Tcl_Interp* interp, Tcl_Object object) {
    SshDestroyInstance(interp, object);
}

bool SshInitSession(Tcl_Interp* interp) {
    static const Tcl_MethodType constructor = {
        .version  = TCL_OO_METHOD_VERSION_CURRENT,
        .name     = "Constructor",
        .callProc = Constructor
    };
    static const Tcl_MethodType handleKeyExchange = {
        .version  = TCL_OO_METHOD_VERSION_CURRENT,
        .name     = "handleKeyExchange",
        .callProc = HandleKeyExchange
    };
    static const Tcl_MethodType setCallback = {
        .callProc = SetCallback,
        .name     = "setCallback",
        .version  = TCL_OO_METHOD_VERSION_CURRENT
    };
    static const Tcl_MethodType* methods[] =
            {&handleKeyExchange, &setCallback, NULL};
    Tcl_Class class = SshNewClass(
            interp, "::ssh::session", &constructor, NULL, methods);

    return (class != NULL);
}

Tcl_Object SshNewSession(Tcl_Interp* interp) {
    Tcl_Object object = SshNewInstance(interp, "::ssh::session", NULL);

    if (object != NULL)
        SshSetInterp(object, interp);

    return object;
}
