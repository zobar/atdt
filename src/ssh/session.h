#pragma once

#include <stdbool.h>
#include <tclOO.h>

bool SshSessionInit(Tcl_Interp* interp);

Tcl_Object SshSessionNew(Tcl_Interp* interp);
