#pragma once

#include "ssh.h"

#include <libssh/server.h>

ssh_bind SshGetBind(Tcl_Interp* interp, Tcl_Object object);

ssh_session SshGetSessionObj(Tcl_Interp* interp, Tcl_Obj* obj);

void SshSetBind(Tcl_Object object, ssh_bind bind);

void SshSetSession(Tcl_Object object, ssh_session channel);
