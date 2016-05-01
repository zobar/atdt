#pragma once

#include <libssh/server.h>
#include <tclOO.h>

ssh_bind SshGetBind(Tcl_Interp* interp, Tcl_Object object);

ssh_session SshGetSession(Tcl_Interp* interp, Tcl_Object object);

void SshSetBind(Tcl_Object object, ssh_bind bind);

void SshSetChannel(Tcl_Object object, Tcl_Channel channel);

void SshSetSession(Tcl_Object object, ssh_session channel);
