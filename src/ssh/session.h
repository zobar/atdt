#pragma once

#include <libssh/libssh.h>
#include <stdbool.h>
#include <tclOO.h>

bool SshSessionInit(Tcl_Interp* interp);

Tcl_Object SshNewSession(Tcl_Interp* interp, ssh_session session);
