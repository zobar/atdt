#include <config.h>
#include <libssh/server.h>
#include <stdbool.h>
#include <stdio.h>
#include <tclOO.h>

static int Eval(Tcl_Interp* interp, int argc, const char* argv[], int flags) {
    int i = 0;
    Tcl_Obj** objv = ckalloc(argc * sizeof(Tcl_Obj*));
    int result = TCL_ERROR;

    for (i = 0; i < argc; ++i) {
        objv[i] = Tcl_NewStringObj(argv[i], -1);
        Tcl_IncrRefCount(objv[i]);
    }

    result = Tcl_EvalObjv(interp, argc, objv, flags);

    for (i = 0; i < argc; ++i)
        Tcl_DecrRefCount(objv[i]);
    ckfree(objv);
    return result;
}

static Tcl_Class NewClass(Tcl_Interp* interp, const char* name) {
    const char* argv[] = {"::oo::class", "create", name};
    Tcl_Class result = NULL;

    if (Eval(interp, 3, argv, TCL_EVAL_GLOBAL) == TCL_OK) {
        Tcl_Object classObject =
                Tcl_GetObjectFromObj(interp, Tcl_GetObjResult(interp));
        if (classObject != NULL)
            result = Tcl_GetObjectAsClass(classObject);
    }

    return result;
}

static int DefineClass(Tcl_Interp* interp, const char* name,
                       const Tcl_MethodType* constructor) {
    Tcl_Class class = NewClass(interp, name);
    int result = TCL_ERROR;

    if (class != NULL) {
        Tcl_ClassSetConstructor(interp, class,
                Tcl_NewMethod(interp, class, NULL, true, constructor, NULL));
        result = TCL_OK;
    }

    return result;
}

static int CloneSshBindMetadata(Tcl_Interp* interp, ClientData srcMetadata,
                                ClientData* dstMetadataPtr) {
    return TCL_ERROR;
}

static void DeleteSshBindMetadata(ClientData metadata) {
    ssh_bind_free(metadata);
}

static const Tcl_ObjectMetadataType SshBindMetadata = {
    .version    = TCL_OO_METADATA_VERSION_CURRENT,
    .name       = "ssh_bind",
    .deleteProc = DeleteSshBindMetadata,
    .cloneProc  = CloneSshBindMetadata
};

static int CallSshBindConstructor(ClientData clientData, Tcl_Interp* interp,
                                  Tcl_ObjectContext objectContext, int objc,
                                  Tcl_Obj* const* objv) {
    int result = TCL_ERROR;
    Tcl_Object self = Tcl_ObjectContextObject(objectContext);

    if (self != NULL) {
        Tcl_ObjectSetMetadata(self, &SshBindMetadata, ssh_bind_new());
        result = TCL_OK;
    }

    return result;
}

static const Tcl_MethodType SshBindConstructor = {
    .version    = TCL_OO_METHOD_VERSION_CURRENT,
    .name       = "Constructor",
    .callProc   = CallSshBindConstructor,
    .deleteProc = NULL,
    .cloneProc  = NULL
};

int Ssh_Init(Tcl_Interp* interp) {
    int result = TCL_ERROR;

    if (Tcl_InitStubs(interp, TCL_VERSION, false)
            && Tcl_OOInitStubs(interp)
            && Tcl_PkgProvide(interp, "ssh", PACKAGE_VERSION) == TCL_OK) {
        result = DefineClass(interp, "::ssh::bind", &SshBindConstructor);
    }

    return result;
}
