#include "metadata.h"
#include "session.h"

static int Constructor(unused ClientData clientData, unused Tcl_Interp* interp,
                       Tcl_ObjectContext objectContext, unused int objc,
                       unused Tcl_Obj* const* objv) {
    int result = TCL_ERROR;
    int skip = Tcl_ObjectContextSkippedArgs(objectContext);

    if (objc == skip) {
        Tcl_Object self = Tcl_ObjectContextObject(objectContext);
        ssh_session session = ssh_new();

        SshSetSession(self, session);
        result = TCL_OK;
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
    static const Tcl_MethodType* methods[] = {NULL};

    return SshNewClass(interp, "::ssh::session", &constructor, methods) != NULL;
}

Tcl_Object SshSessionNew(Tcl_Interp* interp) {
    return SshNewInstance(interp, "::ssh::session", NULL);
}
