#include "sshInt.h"

static int CloneBindMetadata(Tcl_Interp* interp, unused ClientData srcMetadata,
        unused ClientData* dstMetadataPtr) {
    Tcl_SetObjResult(interp, Tcl_NewStringObj("bind is not clonable", -1));

    return TCL_ERROR;
}

static int CloneChannelMetadata(unused Tcl_Interp* interp,
        ClientData srcMetadata, ClientData* dstMetadataPtr) {
    Tcl_RegisterChannel(NULL, srcMetadata);
    *dstMetadataPtr = srcMetadata;

    return TCL_OK;
}

static int CloneInterpMetadata(unused Tcl_Interp* interp,
        ClientData srcMetadata, ClientData* dstMetadataPtr) {
    *dstMetadataPtr = srcMetadata;

    return TCL_OK;
}

static int CloneMessageMetadata(Tcl_Interp* interp,
        unused ClientData srcMetadata, unused ClientData* dstMetadataPtr) {
    Tcl_SetObjResult(interp, Tcl_NewStringObj("message is not clonable", -1));

    return TCL_ERROR;
}

static int ClonePortMetadata(unused Tcl_Interp* interp, ClientData srcMetadata,
        ClientData* dstMetadataPtr) {
    int* srcPort = srcMetadata;
    int** dstPort = (int**) dstMetadataPtr;

    *dstPort = ckalloc(sizeof(int));
    **dstPort = *srcPort;

    return TCL_OK;
}

static int CloneSessionMetadata(Tcl_Interp* interp,
        unused ClientData srcMetadata, unused ClientData* dstMetadataPtr) {
    Tcl_SetObjResult(interp, Tcl_NewStringObj("session is not clonable", -1));

    return TCL_ERROR;
}

static int CloneTclObjMetadata(
        unused Tcl_Interp* interp, ClientData srcMetadata,
        ClientData* dstMetadataPtr) {
    Tcl_IncrRefCount((Tcl_Obj*) srcMetadata);
    *dstMetadataPtr = srcMetadata;

    return TCL_OK;
}

static void DeleteBindMetadata(ClientData metadata) {
    ssh_bind_free(metadata);
}

static void DeleteChannelMetadata(ClientData metadata) {
    Tcl_ClearChannelHandlers(metadata);
    Tcl_UnregisterChannel(NULL, metadata);
}

static void DeleteInterpMetadata(unused ClientData metadata) {
}

static void DeleteMessageMetadata(ClientData metadata) {
    ssh_message_free(metadata);
}

static void DeletePortMetadata(ClientData metadata) {
    ckfree(metadata);
}

static void DeleteSessionMetadata(ClientData metadata) {
    ssh_free(metadata);
}

static void DeleteTclObjMetadata(ClientData metadata) {
    Tcl_DecrRefCount(metadata);
}

static const Tcl_ObjectMetadataType BindMetadata = {
    .cloneProc  = CloneBindMetadata,
    .deleteProc = DeleteBindMetadata,
    .name       = "bind",
    .version    = TCL_OO_METADATA_VERSION_CURRENT
};

static const Tcl_ObjectMetadataType ChannelMetadata = {
    .cloneProc  = CloneChannelMetadata,
    .deleteProc = DeleteChannelMetadata,
    .name       = "channel",
    .version    = TCL_OO_METADATA_VERSION_CURRENT
};

static const Tcl_ObjectMetadataType IncomingConnectionCallbackMetadata = {
    .cloneProc  = CloneTclObjMetadata,
    .deleteProc = DeleteTclObjMetadata,
    .name       = "incomingConnectionCallback",
    .version    = TCL_OO_METADATA_VERSION_CURRENT
};

static const Tcl_ObjectMetadataType InterpMetadata = {
    .cloneProc  = CloneInterpMetadata,
    .deleteProc = DeleteInterpMetadata,
    .name       = "interp",
    .version    = TCL_OO_METADATA_VERSION_CURRENT
};

static const Tcl_ObjectMetadataType MessageMetadata = {
    .cloneProc  = CloneMessageMetadata,
    .deleteProc = DeleteMessageMetadata,
    .name       = "message",
    .version    = TCL_OO_METADATA_VERSION_CURRENT
};

static const Tcl_ObjectMetadataType NoneAuthCallbackMetadata = {
    .cloneProc  = CloneTclObjMetadata,
    .deleteProc = DeleteTclObjMetadata,
    .name       = "noneAuthCallback",
    .version    = TCL_OO_METADATA_VERSION_CURRENT
};

static const Tcl_ObjectMetadataType PortMetadata = {
    .cloneProc  = ClonePortMetadata,
    .deleteProc = DeletePortMetadata,
    .name       = "port",
    .version    = TCL_OO_METADATA_VERSION_CURRENT
};

static const Tcl_ObjectMetadataType SessionMetadata = {
    .cloneProc  = CloneSessionMetadata,
    .deleteProc = DeleteSessionMetadata,
    .name       = "session",
    .version    = TCL_OO_METADATA_VERSION_CURRENT
};

static const Tcl_ObjectMetadataType SessionNameMetadata = {
    .cloneProc  = CloneTclObjMetadata,
    .deleteProc = DeleteTclObjMetadata,
    .name       = "sessionName",
    .version    = TCL_OO_METADATA_VERSION_CURRENT
};

static const Tcl_ObjectMetadataType StatusClosedErrorCallbackMetadata = {
    .cloneProc  = CloneTclObjMetadata,
    .deleteProc = DeleteTclObjMetadata,
    .name       = "statusClosedErrorCallback",
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
    return Get(interp, object, &BindMetadata);
}

Tcl_Channel SshGetChannel(Tcl_Interp* interp, Tcl_Object object) {
    return Get(interp, object, &ChannelMetadata);
}

Tcl_Obj* SshGetIncomingConnectionCallback(
        Tcl_Interp* interp, Tcl_Object object) {
    return Get(interp, object, &IncomingConnectionCallbackMetadata);
}

Tcl_Interp* SshGetInterp(Tcl_Interp* interp, Tcl_Object object) {
    return Get(interp, object, &InterpMetadata);
}

ssh_message SshGetMessage(Tcl_Interp* interp, Tcl_Object object) {
    return Get(interp, object, &MessageMetadata);
}

Tcl_Obj* SshGetNoneAuthCallback(Tcl_Interp* interp, Tcl_Object object) {
    return Get(interp, object, &NoneAuthCallbackMetadata);
}

int SshGetPort(unused Tcl_Interp* interp, Tcl_Object object) {
    int* port = Tcl_ObjectGetMetadata(object, &PortMetadata);
    int result = 22;

    if (port != NULL)
        result = *port;

    return result;
}

ssh_session SshGetSession(Tcl_Interp* interp, Tcl_Object object) {
    return Get(interp, object, &SessionMetadata);
}

Tcl_Obj* SshGetSessionName(Tcl_Interp* interp, Tcl_Object object) {
    return Get(interp, object, &SessionNameMetadata);
}

Tcl_Obj* SshGetStatusClosedErrorCallback(
        Tcl_Interp* interp, Tcl_Object object) {
    return Get(interp, object, &StatusClosedErrorCallbackMetadata);
}

void SshSetBind(Tcl_Object object, ssh_bind bind) {
    Tcl_ObjectSetMetadata(object, &BindMetadata, bind);
}

void SshSetChannel(Tcl_Object object, Tcl_Channel channel) {
    Tcl_RegisterChannel(NULL, channel);
    Tcl_ObjectSetMetadata(object, &ChannelMetadata, channel);
}

void SshSetIncomingConnectionCallback(
        Tcl_Object object, Tcl_Obj* incomingConnectionCallback) {
    Tcl_IncrRefCount(incomingConnectionCallback);
    Tcl_ObjectSetMetadata(
            object, &IncomingConnectionCallbackMetadata,
            incomingConnectionCallback);
}

void SshSetInterp(Tcl_Object object, Tcl_Interp* interp) {
    Tcl_ObjectSetMetadata(object, &InterpMetadata, interp);
}

void SshSetMessage(Tcl_Object object, ssh_message message) {
    Tcl_ObjectSetMetadata(object, &MessageMetadata, message);
}

void SshSetNoneAuthCallback(Tcl_Object object, Tcl_Obj* noneAuthCallback) {
    Tcl_IncrRefCount(noneAuthCallback);
    Tcl_ObjectSetMetadata(object, &NoneAuthCallbackMetadata, noneAuthCallback);
}

void SshSetPort(Tcl_Object object, int port) {
    int* portPtr = ckalloc(sizeof(int));

    *portPtr = port;
    Tcl_ObjectSetMetadata(object, &PortMetadata, portPtr);
}

void SshSetSession(Tcl_Object object, ssh_session session) {
    Tcl_ObjectSetMetadata(object, &SessionMetadata, session);
}

void SshSetSessionName(Tcl_Object object, Tcl_Obj* sessionName) {
    Tcl_IncrRefCount(sessionName);
    Tcl_ObjectSetMetadata(object, &SessionNameMetadata, sessionName);
}

void SshSetStatusClosedErrorCallback(
        Tcl_Object object, Tcl_Obj* statusClosedErrorCallback) {
    Tcl_IncrRefCount(statusClosedErrorCallback);
    Tcl_ObjectSetMetadata(
            object, &StatusClosedErrorCallbackMetadata,
            statusClosedErrorCallback);
}
