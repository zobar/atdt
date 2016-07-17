#include "sshInt.h"

int SshLibError(Tcl_Interp* interp, void* self, int status) {
    int result = TCL_OK;

    if (status != SSH_OK) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(ssh_get_error(self), -1));
        result = TCL_ERROR;
    }

    return result;
}

int SshPosixError(Tcl_Interp* interp, int status) {
    int result = TCL_OK;

    if (status) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(Tcl_ErrnoMsg(status), -1));
        result = TCL_ERROR;
    }

    return result;
}
