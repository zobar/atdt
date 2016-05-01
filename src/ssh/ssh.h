#pragma once

#include <tcl.h>

#define unused __attribute__((__unused__))

int Ssh_Init(Tcl_Interp* interp);
