#include "config.h"
#include "ssh.h"

#include "bind.h"
#include "event.h"
#include "session.h"
#include "thread.h"

#include <stdio.h>

int Ssh_Init(Tcl_Interp* interp) {
    printf("init package\n");
    int result = TCL_ERROR;

    if (Tcl_InitStubs(interp, TCL_VERSION, false)
            && Tcl_OOInitStubs(interp)
            && SshThreadInit(interp)
            && SshEventInit(interp)
            && SshBindInit(interp)
            && SshSessionInit(interp))
        result = Tcl_PkgProvide(interp, "ssh", PACKAGE_VERSION);

    return result;
}
