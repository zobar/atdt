#include "sshInt.h"

static Tcl_Object GetObjectFromName(Tcl_Interp* interp, const char* name) {
    Tcl_Obj* nameObj = Tcl_NewStringObj(name, -1);
    Tcl_Object result = NULL;

    Tcl_IncrRefCount(nameObj);
    result = Tcl_GetObjectFromObj(interp, nameObj);
    Tcl_DecrRefCount(nameObj);

    return result;
}

static Tcl_Class GetClassFromName(Tcl_Interp* interp, const char* name) {
    Tcl_Object classObject = GetObjectFromName(interp, name);
    Tcl_Class result = NULL;

    if (classObject != NULL)
        result = Tcl_GetObjectAsClass(classObject);

    return result;
}

Tcl_Object SshNewInstance(Tcl_Interp* interp, const char* className,
                       const char* name) {
    Tcl_Class class = GetClassFromName(interp, className);
    Tcl_Object result = NULL;

    if (class != NULL)
        result = Tcl_NewObjectInstance(interp, class, name, NULL, 0, NULL, 0);

    return result;
}

Tcl_Class SshNewClass(Tcl_Interp* interp, const char* name,
                      const Tcl_MethodType* constructor,
                      const Tcl_MethodType* destructor,
                      const Tcl_MethodType* methods[]) {
    Tcl_Object classObject = SshNewInstance(interp, "::oo::class", name);
    Tcl_Class result = NULL;

    if (classObject != NULL) {
        result = Tcl_GetObjectAsClass(classObject);
        if (result != NULL) {
            if (constructor != NULL) {
                Tcl_Method method = Tcl_NewMethod(
                        interp, result, NULL, true, constructor, NULL);

                Tcl_ClassSetConstructor(interp, result, method);
            }

            if (destructor != NULL) {
                Tcl_Method method = Tcl_NewMethod(
                        interp, result, NULL, true, destructor, NULL);

                Tcl_ClassSetDestructor(interp, result, method);
            }

            if (methods != NULL) {
                int i = 0;

                for (i = 0; methods[i] != NULL; ++i) {
                    const Tcl_MethodType* method = methods[i];
                    Tcl_Obj* nameObj = Tcl_NewStringObj(methods[i]->name, -1);

                    Tcl_IncrRefCount(nameObj);
                    Tcl_NewMethod(interp, result, nameObj, true, method, NULL);
                    Tcl_DecrRefCount(nameObj);
                }
            }
        }
    }

    return result;
}
