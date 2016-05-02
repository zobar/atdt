#include "config.h"
#include "error.h"

#include <libssh/libssh.h>

int PosixError(Tcl_Interp* interp, int status) {
    int result = TCL_OK;

    if (status) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(Tcl_ErrnoMsg(status), -1));
        result = TCL_ERROR;
    }

    return result;
}

int SshError(Tcl_Interp* interp, void* self, int status) {
    int result = TCL_OK;

    if (status != TCL_OK) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(ssh_get_error(self), -1));
        result = TCL_ERROR;
    }

    return result;
}
