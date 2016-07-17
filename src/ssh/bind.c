#include "sshInt.h"

static int SetPort(Tcl_Interp* interp, Tcl_Object object, Tcl_Obj* arg) {
    int port = 0;
    int result = Tcl_GetIntFromObj(interp, arg, &port);

    if (result != TCL_ERROR)
        SshSetPort(object, port);

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
    enum events {PORT, RSA_KEY};
    static const char* keys[] = {"-port", "-rsakey", NULL};

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

static int Constructor(
        ClientData clientData, Tcl_Interp* interp,
        Tcl_ObjectContext objectContext, int objc, Tcl_Obj* const* objv) {
    ssh_bind bind = ssh_bind_new();
    Tcl_Object object = Tcl_ObjectContextObject(objectContext);

    ssh_bind_set_blocking(bind, false);
    SshSetBind(object, bind);
    SshSetInterp(object, interp);

    return Configure(clientData, interp, objectContext, objc, objv);
}

static void IncomingConnection(
        ClientData clientData, Tcl_Channel channel, unused char* hostName,
        unused int port) {
    int fd = -1;
    Tcl_Object object = clientData;
    ssh_bind bind = SshGetBind(NULL, object);
    Tcl_Obj* callback = SshGetIncomingConnectionCallback(NULL, object);
    Tcl_Interp* interp = SshGetInterp(NULL, object);
    int result = Tcl_GetChannelHandle(channel, TCL_READABLE, (ClientData*) &fd);

    if (bind != NULL && callback != NULL && interp != NULL
            && result == TCL_OK) {
        Tcl_Object sessionObject = NULL;
        ssh_session session = NULL;

        Tcl_Preserve(interp);

        sessionObject = SshNewSession(interp);
        session = SshGetSession(interp, sessionObject);
        SshSetChannel(sessionObject, channel);

        result = SshLibError(
                interp, session, ssh_bind_accept_fd(bind, session, fd));
        if (result == TCL_OK) {
            Tcl_Obj* command[] = {callback, NULL};

            command[1] = Tcl_GetObjectName(interp, sessionObject);
            result = SshCallBack(interp, 2, command);
        }

        if (result != TCL_OK)
            SshDestroySession(interp, sessionObject);

        Tcl_Release(interp);
    }
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
            int port = SshGetPort(interp, object);
            Tcl_Channel channel = Tcl_OpenTcpServer(
                interp, port, NULL, IncomingConnection, object);

            if (channel != NULL) {
                SshSetChannel(object, channel);
                result = TCL_OK;
            }
        }
    }
    else
        Tcl_WrongNumArgs(interp, skip, objv, NULL);

    return result;
}

static int SetCallback(
        unused ClientData clientData, Tcl_Interp* interp,
        Tcl_ObjectContext objectContext, int objc, Tcl_Obj* const* objv) {
    enum options {INCOMING_CONNECTION};
    static const char* keys[] = {"incomingConnection", NULL};

    Tcl_Object object = Tcl_ObjectContextObject(objectContext);
    int result = TCL_ERROR;
    int skip = Tcl_ObjectContextSkippedArgs(objectContext);

    if (objc == skip + 2) {
        Tcl_Obj* callback = objv[skip + 1];
        int event = -1;

        result = Tcl_GetIndexFromObj(
            interp, objv[skip], keys, "event", 0, &event);
        if (result == TCL_OK) {
            switch(event) {
            case INCOMING_CONNECTION:
                SshSetIncomingConnectionCallback(object, callback);
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

bool SshInitBind(Tcl_Interp* interp) {
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
    static const Tcl_MethodType listen = {
        .callProc = Listen,
        .name     = "listen",
        .version  = TCL_OO_METHOD_VERSION_CURRENT
    };
    static const Tcl_MethodType setCallback = {
        .callProc = SetCallback,
        .name     = "setCallback",
        .version  = TCL_OO_METHOD_VERSION_CURRENT
    };
    static const Tcl_MethodType* methods[] =
            {&configure, &listen, &setCallback, NULL};
    Tcl_Class class = SshNewClass(
            interp, "::ssh::bind", &constructor, NULL, methods);

    return (class != NULL);
}
