#pragma once

#include "ssh.h"

#include <stdbool.h>

bool SshSessionInit(Tcl_Interp* interp);

Tcl_Object SshSessionNew(Tcl_Interp* interp);
