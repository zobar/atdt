#pragma once

#include <libssh/libssh.h>
#include <tcl.h>

int SshAddFdEventHandler(Tcl_Interp* interp, socket_t fd, short events,
	                     ssh_event_callback callback, ClientData clientData);

int SshRemoveFdEventHandler(Tcl_Interp* interp, socket_t fd);
