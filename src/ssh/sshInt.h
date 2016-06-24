#pragma once

#include "config.h"
#include "ssh.h"

#include <libssh/server.h>
#include <stdbool.h>
#include <tclOO.h>

#define internal __attribute__((visibility("hidden")))

#define unused __attribute__((__unused__))

internal bool SshBindInit(Tcl_Interp* interp);

internal int SshCallBack(Tcl_Interp* interp, int objc, Tcl_Obj* objv[]);

internal void SshDestroyInstance(Tcl_Interp* interp, Tcl_Object object);

internal void SshDestroySession(Tcl_Interp* interp, Tcl_Object object);

internal ssh_bind SshGetBind(Tcl_Interp* interp, Tcl_Object object);

internal Tcl_Channel SshGetChannel(Tcl_Interp* interp, Tcl_Object object);

internal Tcl_Obj* SshGetIncomingConnectionCallback(
		Tcl_Interp* interp, Tcl_Object object);

internal Tcl_Interp* SshGetInterp(Tcl_Interp* interp, Tcl_Object object);

internal Tcl_Obj* SshGetNoneAuthCallback(Tcl_Interp* interp, Tcl_Object object);

internal int SshGetPort(Tcl_Interp* interp, Tcl_Object object);

internal ssh_session SshGetSession(Tcl_Interp* interp, Tcl_Object object);

internal Tcl_Obj* SshGetStatusClosedErrorCallback(
		Tcl_Interp* interp, Tcl_Object object);

internal int SshLibError(Tcl_Interp* interp, void* self, int status);

internal Tcl_Class SshNewClass(Tcl_Interp* interp, const char* name,
        const Tcl_MethodType* constructor, const Tcl_MethodType* destructor,
        const Tcl_MethodType* methods[]);

internal Tcl_Object SshNewInstance(Tcl_Interp* interp, const char* className,
        const char* name);

internal Tcl_Object SshNewSession(Tcl_Interp* interp);

internal int SshPosixError(Tcl_Interp* interp, int errno);

internal bool SshSessionInit(Tcl_Interp* interp);

internal void SshSetBind(Tcl_Object object, ssh_bind bind);

internal void SshSetChannel(Tcl_Object object, Tcl_Channel channel);

internal void SshSetIncomingConnectionCallback(
		Tcl_Object object, Tcl_Obj* incomingConnectionCallback);

internal void SshSetInterp(Tcl_Object object, Tcl_Interp* interp);

internal void SshSetNoneAuthCallback(
		Tcl_Object object, Tcl_Obj* noneAuthCallback);

internal void SshSetPort(Tcl_Object object, int port);

internal void SshSetSession(Tcl_Object object, ssh_session session);

internal void SshSetStatusClosedErrorCallback(
		Tcl_Object object, Tcl_Obj* incomingConnectionCallback);

internal bool SshThreadInit(Tcl_Interp* interp);
