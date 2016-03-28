#include "bind.h"
#include "metadata.h"

/*
static void AcceptSsh(ClientData clientData, Tcl_Channel channel,
                      char* hostname, int port) {
    SshBind* bind = (SshBind*) clientData;
    Tcl_Interp* interp = bind->interp;
    intptr_t in = 0;
    intptr_t out = 0;

    if (Tcl_GetChannelHandle(channel, TCL_READABLE, (void*) &in) == TCL_OK
            && Tcl_GetChannelHandle(channel, TCL_WRITABLE, (void*) &out) == TCL_OK
            && in == out) {
        Tcl_Object session = SshSessionNew(interp);

        printf("Got channel, read=%i write=%i\n", (int) in, (int) out);
        ssh_bind_accept_fd(bind->bind, SshSessionGetSession(session), in);
    }
    else
        Tcl_Close(interp, channel);
}
*/

static int Accept(unused ClientData clientData, Tcl_Interp* interp,
                  Tcl_ObjectContext objectContext, int objc,
                  Tcl_Obj* const* objv) {
    ssh_bind bind = SshGetBind(interp, Tcl_ObjectContextObject(objectContext));
    int result = TCL_ERROR;
    int skip = Tcl_ObjectContextSkippedArgs(objectContext);

    if (objc == skip + 2) {
        ssh_session session = SshGetSessionObj(interp, objv[skip]);
        if (session) {
            Tcl_Channel channel = Tcl_GetChannel(interp,
                                                 Tcl_GetString(objv[skip + 1]),
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

static int Constructor(unused ClientData clientData, Tcl_Interp* interp,
                       Tcl_ObjectContext objectContext, int objc,
                       Tcl_Obj* const* objv) {
    int result = TCL_ERROR;
    int skip = Tcl_ObjectContextSkippedArgs(objectContext);

    if (objc == skip) {
        ssh_bind bind = ssh_bind_new();
        Tcl_Object self = Tcl_ObjectContextObject(objectContext);

        SshSetBind(self, bind);
        result = TCL_OK;
    }
    else
        Tcl_WrongNumArgs(interp, skip, objv, NULL);

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
    static const Tcl_MethodType* methods[] = {&accept, NULL};

    return SshNewClass(interp, "::ssh::bind", &constructor, methods) != NULL;
}
