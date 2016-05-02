#pragma once

#include <libssh/libssh.h>
#include <stdbool.h>
#include <tcl.h>

int SshAddFdEvent(Tcl_Interp* interp, socket_t fd, short events,
				  ssh_event_callback cb, void* userdata);
