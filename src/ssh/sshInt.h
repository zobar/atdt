#pragma once

#include "config.h"
#include "ssh.h"

#include <libssh/server.h>
#include <stdbool.h>
#include <tclOO.h>

#define unused __attribute__((__unused__))

int SshAddFdEventHandler(Tcl_Interp* interp, socket_t fd, short events,
		ssh_event_callback callback, ClientData clientData);

bool SshBindInit(Tcl_Interp* interp);

ssh_bind SshGetBind(Tcl_Interp* interp, Tcl_Object object);

Tcl_Obj* SshGetConnect(Tcl_Interp* interp, Tcl_Object object);

Tcl_Interp* SshGetInterp(Tcl_Interp* interp, Tcl_Object object);

ssh_session SshGetSession(Tcl_Interp* interp, Tcl_Object object);

Tcl_ThreadId SshGetThreadId(Tcl_Interp* interp, Tcl_Object object);

int SshLibError(Tcl_Interp* interp, void* self, int status);

Tcl_Class SshNewClass(Tcl_Interp* interp, const char* name,
                      const Tcl_MethodType* constructor,
                      const Tcl_MethodType* destructor,
                      const Tcl_MethodType* methods[]);

Tcl_Object SshNewInstance(Tcl_Interp* interp, const char* className,
                          const char* name);

Tcl_Object SshNewSession(Tcl_Interp* interp, ssh_session session);

int SshPosixError(Tcl_Interp* interp, int errno);

int SshRemoveFdEventHandler(Tcl_Interp* interp, socket_t fd);

bool SshSessionInit(Tcl_Interp* interp);

void SshSetBind(Tcl_Object object, ssh_bind bind);

void SshSetConnect(Tcl_Object object, Tcl_Obj* connect);

void SshSetInterp(Tcl_Object object, Tcl_Interp* interp);

void SshSetSession(Tcl_Object object, ssh_session session);

void SshSetThreadId(Tcl_Object object, Tcl_ThreadId threadId);

bool SshThreadInit(Tcl_Interp* interp);
