#include "sshInt.h"

static int CloneBindMetadata(
        Tcl_Interp* interp, unused ClientData srcMetadata,
        unused ClientData* dstMetadataPtr) {
    Tcl_SetObjResult(interp, Tcl_NewStringObj("bind is not clonable", -1));

    return TCL_ERROR;
}

static int CloneConnectMetadata(
        unused Tcl_Interp* interp, ClientData srcMetadata,
        ClientData* dstMetadataPtr) {
    Tcl_IncrRefCount((Tcl_Obj*) srcMetadata);
    *dstMetadataPtr = srcMetadata;

    return TCL_OK;
}

static int CloneInterpMetadata(unused Tcl_Interp* interp,
        ClientData srcMetadata, ClientData* dstMetadataPtr) {
    *dstMetadataPtr = srcMetadata;

    return TCL_OK;
}

static int CloneSessionMetadata(
        Tcl_Interp* interp, unused ClientData srcMetadata,
        unused ClientData* dstMetadataPtr) {
    Tcl_SetObjResult(interp, Tcl_NewStringObj("session is not clonable", -1));

    return TCL_ERROR;
}

static int CloneThreadIdMetadata(
        unused Tcl_Interp* interp, ClientData srcMetadata,
        ClientData* dstMetadataPtr) {
    *dstMetadataPtr = srcMetadata;

    return TCL_OK;
}

static void DeleteBindMetadata(ClientData metadata) {
    ssh_bind_free((ssh_bind) metadata);
}

static void DeleteConnectMetadata(ClientData metadata) {
    Tcl_DecrRefCount((Tcl_Obj*) metadata);
}

static void DeleteInterpMetadata(unused ClientData metadata) {
}

static void DeleteSessionMetadata(ClientData metadata) {
    ssh_free((ssh_session) metadata);
}

static void DeleteThreadIdMetadata(unused ClientData metadata) {
}

static const Tcl_ObjectMetadataType BindMetadata = {
    .cloneProc  = CloneBindMetadata,
    .deleteProc = DeleteBindMetadata,
    .name       = "bind",
    .version    = TCL_OO_METADATA_VERSION_CURRENT
};

static const Tcl_ObjectMetadataType ConnectMetadata = {
    .cloneProc  = CloneConnectMetadata,
    .deleteProc = DeleteConnectMetadata,
    .name       = "connect",
    .version    = TCL_OO_METADATA_VERSION_CURRENT
};

static const Tcl_ObjectMetadataType InterpMetadata = {
    .cloneProc = CloneInterpMetadata,
    .deleteProc = DeleteInterpMetadata,
    .name       = "interp",
    .version    = TCL_OO_METADATA_VERSION_CURRENT
};

static const Tcl_ObjectMetadataType SessionMetadata = {
    .cloneProc  = CloneSessionMetadata,
    .deleteProc = DeleteSessionMetadata,
    .name       = "session",
    .version    = TCL_OO_METADATA_VERSION_CURRENT
};

static const Tcl_ObjectMetadataType ThreadIdMetadata = {
    .cloneProc  = CloneThreadIdMetadata,
    .deleteProc = DeleteThreadIdMetadata,
    .name       = "threadId",
    .version    = TCL_OO_METADATA_VERSION_CURRENT
};

static ClientData Get(
        Tcl_Interp* interp, Tcl_Object object,
        const Tcl_ObjectMetadataType* metaTypePtr) {
    ClientData result = Tcl_ObjectGetMetadata(object, metaTypePtr);

    if (result == NULL && interp != NULL) {
        Tcl_Obj* message = Tcl_ObjPrintf(
                "%s does not have '%s' set",
                Tcl_GetString(Tcl_GetObjectName(interp, object)),
                metaTypePtr->name);

        Tcl_SetObjResult(interp, message);
    }

    return result;
}

ssh_bind SshGetBind(Tcl_Interp* interp, Tcl_Object object) {
    return (ssh_bind) Get(interp, object, &BindMetadata);
}

Tcl_Obj* SshGetConnect(Tcl_Interp* interp, Tcl_Object object) {
    return (Tcl_Obj*) Get(interp, object, &ConnectMetadata);
}

Tcl_Interp* SshGetInterp(Tcl_Interp* interp, Tcl_Object object) {
    return (Tcl_Interp*) Get(interp, object, &InterpMetadata);
}

ssh_session SshGetSession(Tcl_Interp* interp, Tcl_Object object) {
    return (ssh_session) Get(interp, object, &SessionMetadata);
}

Tcl_ThreadId SshGetThreadId(Tcl_Interp* interp, Tcl_Object object) {
    return (Tcl_ThreadId) Get(interp, object, &ThreadIdMetadata);
}

void SshSetBind(Tcl_Object object, ssh_bind bind) {
    Tcl_ObjectSetMetadata(object, &BindMetadata, (ClientData) bind);
}

void SshSetConnect(Tcl_Object object, Tcl_Obj* connect) {
    Tcl_IncrRefCount(connect);
    Tcl_ObjectSetMetadata(object, &ConnectMetadata, (ClientData) connect);
}

void SshSetInterp(Tcl_Object object, Tcl_Interp* interp) {
    Tcl_ObjectSetMetadata(object, &InterpMetadata, (ClientData) interp);
}

void SshSetSession(Tcl_Object object, ssh_session session) {
    Tcl_ObjectSetMetadata(object, &SessionMetadata, (ClientData) session);
}

void SshSetThreadId(Tcl_Object object, Tcl_ThreadId threadId) {
    Tcl_ObjectSetMetadata(object, &ThreadIdMetadata, (ClientData) threadId);
}
