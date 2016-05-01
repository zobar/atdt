#include "config.h"
#include "session.h"

#include "metadata.h"
#include "oo.h"
#include "ssh.h"

static int Configure(unused ClientData clientData, Tcl_Interp* interp,
                     Tcl_ObjectContext objectContext, int objc,
                     Tcl_Obj* const* objv) {
    enum options {BLOCKING};
    static const char* keys[] = {"-blocking", NULL};

    int i = 0;
    int result = TCL_OK;
    ssh_session session = SshGetSession(interp,
                                        Tcl_ObjectContextObject(objectContext));
    int skip = Tcl_ObjectContextSkippedArgs(objectContext);

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
                        if (result != TCL_ERROR)
                            ssh_set_blocking(session, blocking);
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
    SshSetSession(Tcl_ObjectContextObject(objectContext), ssh_new());

    return Configure(clientData, interp, objectContext, objc, objv);
}

static int GetMessage(unused ClientData clientData, Tcl_Interp* interp,
		      Tcl_ObjectContext objectContext, int objc,
		      Tcl_Obj* const* objv) {
    int result = TCL_ERROR;
    int skip = Tcl_ObjectContextSkippedArgs(objectContext);

    if (skip == objc) {
	Tcl_Object self = Tcl_ObjectContextObject(objectContext);
	ssh_session session = SshGetSession(interp, self);

	if (session != NULL) {
	    ssh_message message = ssh_message_get(session);
	    if (message != NULL) {
		printf("Got message, type=%i subtype=%i\n",
		       ssh_message_type(message), ssh_message_subtype(message));
		result = TCL_OK;
	    }
	    else {
		Tcl_SetObjResult(interp,
				 Tcl_NewStringObj(ssh_get_error(session), -1));
	    }
	}
    }
    else
	Tcl_WrongNumArgs(interp, skip, objv, NULL);

    return result;
}

static int HandleKeyExchange(unused ClientData clientData, Tcl_Interp* interp,
                             Tcl_ObjectContext objectContext, int objc,
                             Tcl_Obj* const* objv) {
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
                Tcl_SetObjResult(interp,
                                 Tcl_NewStringObj(ssh_get_error(session), -1));
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
    static const Tcl_MethodType getMessage = {
        .version  = TCL_OO_METHOD_VERSION_CURRENT,
        .name     = "getMessage",
        .callProc = GetMessage
    };
    static const Tcl_MethodType handleKeyExchange = {
        .version  = TCL_OO_METHOD_VERSION_CURRENT,
        .name     = "handleKeyExchange",
        .callProc = HandleKeyExchange
    };
    static const Tcl_MethodType* methods[] =
            {&configure, &getMessage, &handleKeyExchange, NULL};

    return SshNewClass(interp, "::ssh::session", &constructor, methods) != NULL;
}

Tcl_Object SshSessionNew(Tcl_Interp* interp) {
    return SshNewInstance(interp, "::ssh::session", NULL);
}
