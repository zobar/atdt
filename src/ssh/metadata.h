#pragma once

#include <libssh/server.h>
#include <tclOO.h>

ssh_bind SshGetBind(Tcl_Interp* interp, Tcl_Object object);

Tcl_Obj* SshGetConnect(Tcl_Interp* interp, Tcl_Object object);

Tcl_Interp* SshGetInterp(Tcl_Interp* interp, Tcl_Object object);

ssh_session SshGetSession(Tcl_Interp* interp, Tcl_Object object);

Tcl_ThreadId SshGetThreadId(Tcl_Interp* interp, Tcl_Object object);

void SshSetBind(Tcl_Object object, ssh_bind bind);

void SshSetConnect(Tcl_Object object, Tcl_Obj* connect);

void SshSetInterp(Tcl_Object object, Tcl_Interp* interp);

void SshSetSession(Tcl_Object object, ssh_session session);

void SshSetThreadId(Tcl_Object object, Tcl_ThreadId threadId);
