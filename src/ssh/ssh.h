#pragma once

#include "config.h"
#include <tclOO.h>

#define unused __attribute__((__unused__))

int Ssh_Init(Tcl_Interp* interp);

Tcl_Class SshNewClass(Tcl_Interp* interp, const char* name,
                      const Tcl_MethodType* constructor,
                      const Tcl_MethodType* methods[]);

Tcl_Object SshNewInstance(Tcl_Interp* interp, const char* className,
                          const char* name);
