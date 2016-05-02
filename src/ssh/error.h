#pragma once

#include <tcl.h>

int PosixError(Tcl_Interp* interp, int errno);
int SshError(Tcl_Interp* interp, void* self, int status);
