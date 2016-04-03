#include "metadata.h"

static ClientData Get(Tcl_Interp* interp, Tcl_Object object,
                      const Tcl_ObjectMetadataType* metaTypePtr) {
    ClientData result = Tcl_ObjectGetMetadata(object, metaTypePtr);

    if (result == NULL) {
        Tcl_SetObjResult(interp,
                         Tcl_ObjPrintf("%s is not a %s",
                                       Tcl_GetString(Tcl_GetObjectName(interp,
                                                                       object)),
                                       metaTypePtr->name));
    }

    return result;
}

static int CloneBindMetadata(Tcl_Interp* interp, unused ClientData srcMetadata,
                             unused ClientData* dstMetadataPtr) {
    Tcl_SetObjResult(interp, Tcl_NewStringObj("bind is not clonable", -1));
    return TCL_ERROR;
}

static void DeleteBindMetadata(ClientData metadata) {
    ssh_bind_free((ssh_bind) metadata);
}

static const Tcl_ObjectMetadataType BindMetadata = {
    .version    = TCL_OO_METADATA_VERSION_CURRENT,
    .name       = "bind",
    .deleteProc = DeleteBindMetadata,
    .cloneProc  = CloneBindMetadata
};

ssh_bind SshGetBind(Tcl_Interp* interp, Tcl_Object object) {
    return (ssh_bind) Get(interp, object, &BindMetadata);
}

void SshSetBind(Tcl_Object object, ssh_bind bind) {
    Tcl_ObjectSetMetadata(object, &BindMetadata, (ClientData) bind);
}

static int CloneChannelMetadata(Tcl_Interp* interp,
                                unused ClientData srcMetadata,
                                unused ClientData* dstMetadataPtr) {
    Tcl_SetObjResult(interp, Tcl_NewStringObj("channel is not clonable", -1));
    return TCL_ERROR;
}

static void DeleteChannelMetadata(ClientData metadata) {
    Tcl_UnregisterChannel(NULL, (Tcl_Channel) metadata);
}

static const Tcl_ObjectMetadataType ChannelMetadata = {
    .version    = TCL_OO_METADATA_VERSION_CURRENT,
    .name       = "channel",
    .deleteProc = DeleteChannelMetadata,
    .cloneProc  = CloneChannelMetadata
};

void SshSetChannel(Tcl_Object object, Tcl_Channel channel) {
    Tcl_RegisterChannel(NULL, channel);
    Tcl_ObjectSetMetadata(object, &ChannelMetadata, (ClientData) channel);
}

static int CloneSessionMetadata(Tcl_Interp* interp,
                                unused ClientData srcMetadata,
                                unused ClientData* dstMetadataPtr) {
    Tcl_SetObjResult(interp, Tcl_NewStringObj("session is not clonable", -1));
    return TCL_ERROR;
}

static void DeleteSessionMetadata(ClientData metadata) {
    ssh_free((ssh_session) metadata);
}

static const Tcl_ObjectMetadataType SessionMetadata = {
    .version    = TCL_OO_METADATA_VERSION_CURRENT,
    .name       = "session",
    .deleteProc = DeleteSessionMetadata,
    .cloneProc  = CloneSessionMetadata
};

ssh_session SshGetSession(Tcl_Interp* interp, Tcl_Object object) {
    return (ssh_session) Get(interp, object, &SessionMetadata);
}

void SshSetSession(Tcl_Object object, ssh_session session) {
    Tcl_ObjectSetMetadata(object, &SessionMetadata, (ClientData) session);
}
