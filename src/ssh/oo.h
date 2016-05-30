#pragma once

#include <tclOO.h>

Tcl_Class SshNewClass(Tcl_Interp* interp, const char* name,
                      const Tcl_MethodType* constructor,
                      const Tcl_MethodType* destructor,
                      const Tcl_MethodType* methods[]);

Tcl_Object SshNewInstance(Tcl_Interp* interp, const char* className,
                          const char* name);
