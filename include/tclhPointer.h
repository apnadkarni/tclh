#ifndef TCLHPOINTER_H
#define TCLHPOINTER_H

/*
 * Copyright (c) 2021-2023, Ashok P. Nadkarni
 * All rights reserved.
 *
 * See the file LICENSE for license
 */

#include "tclhBase.h"
#include "tclhHash.h"
#include <stdarg.h>

/* Typedef: Tclh_PointerRegistry
 *
 * See <Registered Pointers>
 */
typedef struct TclhPointerRegistryInfo *Tclh_PointerRegistry;

/* Typedef: Tclh_PointerTypeTag
 *
 * See <Pointer Type Tags>.
 */
typedef Tcl_Obj *Tclh_PointerTypeTag;


/* Section: Registered Pointers
 *
 * Provides a facility for safely passing pointers, Windows handles etc. to the
 * Tcl script level. The intent of pointer registration is to make use of
 * pointers passed to script level more robust by preventing errors such as use
 * after free, the wrong pointer type etc. Each pointer is also optionally typed
 * with a tag and verification can check not only that the pointer is registered
 * but has the right type tag.
 *
 * The function <Tclh_PointerLibInit> must be called before any other functions
 * from this library. This must be done in the extension's initialization
 * function for every interpreter in which the extension is loaded.
 *
 * Pointers can be registered as valid with <Tclh_PointerRegister> before being
 * passed up to the script. When passed in from a script, their validity can be
 * checked with <Tclh_PointerVerify>. Pointers should be marked invalid as
 * appropriate by unregistering them with <Tclh_PointerUnregister> or
 * alternatively <Tclh_PointerObjUnregister>. For convenience, when a pointer
 * may match one of several types, the <Tclh_PointerVerifyAnyOf> and
 * <Tclh_PointerObjVerifyAnyOf> take a variable number of type tags.
 *
 * If pointer registration is not deemed necessary (dangerous), the functions
 * <Tclh_PointerWrap> and <Tclh_PointerUnwrapTagged> can be used to convert pointers
 * to and from Tcl_Obj values.
 *
 * The pointer registry is by default shared by all extensions and application
 * within an interpreter. If the embedded wants a private registry, the
 * preprocessor definition TCLH_POINTER_REGISTRY_NAME should be defined
 * (as a string) before inclusion of the header.
 *
 * Section: Pointer Type Tags
 *
 * Pointers are optionally associated with a type using a type tag
 * so that when checking arguments, the pointer type tag can be checked as
 * well. The type tag is typedefed as Tcl's *Tcl_Obj* type and is
 * treated as opaque as far as this library is concerned.
 *
 * As a special case, no type checking is done on pointers with a type tag of
 * NULL.
 */

/* Function: Tclh_PointerLibInit
 * Must be called to initialize the Pointer module before any of
 * the other functions in the module.
 *
 * Parameters:
 * interp - Tcl interpreter in which to initialize.
 * ptrRegP - Location to store reference to the pointer registry. May be NULL.
 *
 * Any allocated resource are automatically freed up when the interpreter
 * is deleted.
 *
 * Returns:
 * TCL_OK    - Library was successfully initialized.
 * TCL_ERROR - Initialization failed. Library functions must not be called.
 *             An error message is left in the interpreter result.
 */
Tclh_ReturnCode Tclh_PointerLibInit(Tcl_Interp *interp,
                                    Tclh_PointerRegistry *ptrRegP);

/* Function: Tclh_PointerRegister
 * Registers a pointer value as being "valid".
 *
 * Parameters:
 * interp  - Tcl interpreter in which the pointer is to be registered.
 *           May be NULL.
 * registryP - pointer registry. If NULL, the registry associated with the
 *             interpreter is used.
 * pointer - Pointer value to be registered.
 * tag     - Type tag for the pointer. Pass NULL or 0 for typeless pointers.
 * objPP   - if not NULL, a pointer to a new Tcl_Obj holding the pointer
 *           representation is stored here on success. The Tcl_Obj has
 *           a reference count of 0.
 *
 * At least one of interp and registryP must be non-NULL.
 *
 * The validity of a registered pointer can then be tested by with
 * <Tclh_PointerVerify> and reversed by calling <Tclh_PointerUnregister>.
 * Registering a pointer that is already registered will raise an error if
 * the tags do not match or if the previous registration was for a counted
 * pointer. Otherwise the duplicate registration is a no-op and the pointer
 * is unregistered on the next call to <Tclh_PointerUnregister> no matter
 * how many times it has been registered.
 *
 * Returns:
 * TCL_OK    - pointer was successfully registered
 * TCL_ERROR - pointer registration failed. An error message is stored in
 *             the interpreter.
 */
Tclh_ReturnCode Tclh_PointerRegister(Tcl_Interp *interp,
                                     Tclh_PointerRegistry registryP,
                                     void *pointer,
                                     Tclh_PointerTypeTag tag,
                                     Tcl_Obj **objPP);

/* Function: Tclh_PointerRegisterCounted
 * Registers a pointer value as being "valid" permitting multiple registrations
 * and unregistrations for the same pointer.
 *
 * Parameters:
 * interp  - Tcl interpreter in which the pointer is to be registered.
 *           May be NULL.
 * registryP - pointer registry. If NULL, the registry associated with the
 *             interpreter is used.
 * pointer - Pointer value to be registered.
 * pointer - Pointer value to be registered.
 * tag     - Type tag for the pointer. Pass NULL or 0 for typeless pointers.
 * objPP   - if not NULL, a pointer to a new Tcl_Obj holding the pointer
 *           representation is stored here on success. The Tcl_Obj has
 *           a reference count of 0.
 *
 * At least one of interp and registryP must be non-NULL.
 *
 * The validity of a registered pointer can then be tested by with
 * <Tclh_PointerVerify> and reversed by calling <Tclh_PointerUnregister>.
 * A counted pointer that is registered multiple times will be treated
 * as valid until the same number of calls to <Tclh_PointerUnregister>.
 *
 * Registering a pointer that is already registered will raise an error if
 * the tags do not match or if the previous registration was for an uncounted
 * pointer.
 *
 * Returns:
 * TCL_OK    - pointer was successfully registered
 * TCL_ERROR - pointer registration failed. An error message is stored in
 *             the interpreter.
 */
Tclh_ReturnCode Tclh_PointerRegisterCounted(Tcl_Interp *interp,
                                            Tclh_PointerRegistry registryP,
                                            void *pointer,
                                            Tclh_PointerTypeTag tag,
                                            Tcl_Obj **objPP);

/* Function: Tclh_PointerUnregister
 * Unregisters a previously registered pointer.
 *
 * Parameters:
 * interp   - Tcl interpreter in which the pointer is to be unregistered.
 *           May be NULL.
 * registryP - pointer registry. If NULL, the registry associated with the
 *             interpreter is used.
 * pointer  - Pointer value to be unregistered.
 * expected_tag  - Type tag for the pointer.
 *
 * At least one of interp and registryP must be non-NULL.
 *
 * The pointer may have been registered either via <Tclh_PointerRegister> or
 * <Tclh_PointerRegisterCounted>. In the former case, the pointer becomes
 * immediately inaccessible (as defined by Tclh_PointerVerify). In the latter
 * case, it will become inaccessible if it has been unregistered as many times
 * as it has been registered.
 *
 * Returns:
 * TCL_OK    - The pointer was successfully unregistered.
 * TCL_ERROR - The pointer was not registered or was registered with a
 *             different type. An error message is left in interp.
 */
Tclh_ReturnCode Tclh_PointerUnregister(Tcl_Interp *interp,
                                       Tclh_PointerRegistry registryP,
                                       const void *pointer,
                                       Tclh_PointerTypeTag expected_tag);

/* Function: Tclh_PointerVerify
 * Verifies that the passed pointer is registered as a valid pointer
 * of a given type.
 *
 * interp   - Tcl interpreter in which the pointer is to be verified.
 *           May be NULL.
 * registryP - pointer registry. If NULL, the registry associated with the
 *             interpreter is used.
 * pointer  - Pointer value to be verified.
 * expected_tag - Type tag for the pointer. If *NULL*, the pointer registration
 *                is verified but its tag is not checked.
 *
 * At least one of interp and registryP must be non-NULL.
 *
 * Returns:
 * TCL_OK    - The pointer is registered with the same type tag.
 * TCL_ERROR - Pointer is unregistered or a different type. An error message
 *             is stored in interp.
 */
Tclh_ReturnCode Tclh_PointerVerify(Tcl_Interp *interp,
                                   Tclh_PointerRegistry registryP,
                                   const void *voidP,
                                   Tclh_PointerTypeTag expected_tag);

/* Function: Tclh_PointerObjGetTag
 * Returns the pointer type tag for a Tcl_Obj pointer wrapper.
 *
 * Parameters:
 * interp - Interpreter to store errors if any. May be NULL.
 * objP   - Tcl_Obj holding the wrapped pointer.
 * tagPtr - Location to store the type tag. Note the reference count is *not*
 *          incremented. Caller should do that if it wants to preserve it.
 *
 * Returns:
 * TCL_OK    - objP holds a valid pointer. *tagPtr will hold the type tag.
 * TCL_ERROR - objP is not a wrapped pointer. interp, if not NULL, will hold
 *             error message.
 */
Tclh_ReturnCode Tclh_PointerObjGetTag(Tcl_Interp *interp,
                                      Tcl_Obj *objP,
                                      Tclh_PointerTypeTag *tagPtr);

/* Function: Tclh_PointerObjUnregister
 * Unregisters a previously registered pointer passed in as a Tcl_Obj.
 * Null pointers are silently ignored without an error being raised.
 *
 * Parameters:
 * interp   - Tcl interpreter in which the pointer is to be unregistered.
 *            May be NULL.
 * registryP - pointer registry. If NULL, the registry associated with the
 *             interpreter is used.
 * objP     - Tcl_Obj containing a pointer value to be unregistered.
 * pointerP - If not NULL, the pointer value from objP is stored here
 *            on success.
 * tagPtr   - Type tag for the pointer. May be 0 or NULL if pointer
 *            type is not to be checked.
 *
 * At least one of interp and registryP must be non-NULL if tagPtr is not NULL.
 *
 * Returns:
 * TCL_OK    - The pointer was successfully unregistered.
 * TCL_ERROR - The pointer was not registered or was registered with a
 *             different type. An error message is left in interp.
 */
Tclh_ReturnCode Tclh_PointerObjUnregister(Tcl_Interp *interp,
                                          Tclh_PointerRegistry registryP,
                                          Tcl_Obj *objP,
                                          void **pointerP,
                                          Tclh_PointerTypeTag tag);

/* Function: Tclh_PointerObjUnregisterAnyOf
 * Unregisters a previously registered pointer passed in as a Tcl_Obj
 * after checking it is one of the specified types.
 *
 * Parameters:
 * interp   - Tcl interpreter in which the pointer is to be unregistered.
 *            May be NULL.
 * registryP - pointer registry. If NULL, the registry associated with the
 *             interpreter is used.
 * objP     - Tcl_Obj containing a pointer value to be unregistered.
 * pointerP - If not NULL, the pointer value from objP is stored here
 *            on success.
 * tag      - Type tag for the pointer. May be 0 or NULL if pointer
 *            type is not to be checked.
 *
 * At least one of interp and registryP must be non-NULL.
 *
 * Returns:
 * TCL_OK    - The pointer was successfully unregistered.
 * TCL_ERROR - The pointer was not registered or was registered with a
 *             different type. An error message is left in interp.
 */
Tclh_ReturnCode Tclh_PointerObjUnregisterAnyOf(Tcl_Interp *interp,
                                               Tclh_PointerRegistry registryP,
                                               Tcl_Obj *objP,
                                               void **pointerP,
                                               ... /* tag, ... , NULL */);

/* Function: Tclh_PointerObjVerify
 * Verifies a Tcl_Obj contains a wrapped pointer that is registered
 * and, optionally, of a specified type.
 *
 * Parameters:
 * interp   - Tcl interpreter in which the pointer is to be verified.
 *            May be NULL.
 * registryP - pointer registry. If NULL, the registry associated with the
 *             interpreter is used.
 * objP     - Tcl_Obj containing a pointer value to be verified.
 * pointerP - If not NULL, the pointer value from objP is stored here
 *            on success.
 * expected_tag - Type tag for the pointer. May be *NULL* if type is not
 *                to be checked.
 *
 * At least one of interp and registryP must be non-NULL.
 *
 * Returns:
 * TCL_OK    - The pointer was successfully verified.
 * TCL_ERROR - The pointer was not registered or was registered with a
 *             different type. An error message is left in interp.
 */
Tclh_ReturnCode Tclh_PointerObjVerify(Tcl_Interp *interp,
                                      Tclh_PointerRegistry registryP,
                                      Tcl_Obj *objP,
                                      void **pointerP,
                                      Tclh_PointerTypeTag expected_tag);

/* Function: Tclh_PointerObjVerifyAnyOf
 * Verifies a Tcl_Obj contains a wrapped pointer that is registered
 * and one of several allowed types.
 *
 * Parameters:
 * interp   - Tcl interpreter in which the pointer is to be verified.
 *            May be NULL.
 * registryP - pointer registry. If NULL, the registry associated with the
 *             interpreter is used.
 * objP     - Tcl_Obj containing a pointer value to be verified.
 * pointerP - If not NULL, the pointer value from objP is stored here
 *            on success.
 * ...      - Remaining arguments are type tags terminated by a NULL argument.
 *            The pointer must be one of these types.
 *
 * At least one of interp and registryP must be non-NULL.
 *
 * Returns:
 * TCL_OK    - The pointer was successfully verified.
 * TCL_ERROR - The pointer was not registered or was registered with a
 *             type that is not one of the passed ones.
 *             An error message is left in interp.
 */
Tclh_ReturnCode Tclh_PointerObjVerifyAnyOf(Tcl_Interp *interp,
                                           Tclh_PointerRegistry registryP,
                                           Tcl_Obj *objP,
                                           void **pointerP,
                                           ... /* tag, tag, NULL */);

/* Function: Tclh_PointerWrap
 * Wraps a pointer into a Tcl_Obj.
 * The passed pointer is not registered as a valid pointer, nor is
 * any check made that it was previously registered.
 *
 * Parameters:
 * pointer  - Pointer value to be wrapped.
 * tag      - Type tag for the pointer. May be 0 or NULL if pointer
 *            is typeless.
 *
 * Returns:
 * Pointer to a Tcl_Obj with reference count 0.
 */
Tcl_Obj *Tclh_PointerWrap(void *pointer, Tclh_PointerTypeTag tag);

/* Function: Tclh_PointerUnwrap
 * Unwraps a Tcl_Obj representing a pointer. No checks are made with respect
 * to any tag or registration.
 *
 * Parameters:
 * interp - Interpreter in which to store error messages. May be NULL.
 * objP   - Tcl_Obj holding the wrapped pointer value.
 * pointerP - if not NULL, location to store unwrapped pointer.
 *
 * Returns:
 * TCL_OK    - Success, with the unwrapped pointer stored in *pointerP.
 * TCL_ERROR - Failure, with interp containing error message.
 */
Tclh_ReturnCode
Tclh_PointerUnwrap(Tcl_Interp *interp, Tcl_Obj *objP, void **pointerP);

/* Function: Tclh_PointerUnwrapTagged
 * Unwraps a Tcl_Obj representing a pointer checking it is of the
 * expected type. No checks are made with respect to its registration.
 *
 * Parameters:
 * interp - Interpreter in which to store error messages. May be NULL.
 * registryP - pointer registry. If NULL, the registry associated with the
 *             interpreter is used.
 * objP   - Tcl_Obj holding the wrapped pointer value.
 * pointerP - if not NULL, location to store unwrapped pointer.
 * expected_tag - Type tag for the pointer. May be *NULL* if type is not
 *                to be checked. If not NULL, at least one of interp or
 *                registryP must not be NULL.
 *
 * The function does not check tags if the unwrapped pointer is NULL and 
 * is also untagged.
 *
 * Returns:
 * TCL_OK    - Success, with the unwrapped pointer stored in *pointerP.
 * TCL_ERROR - Failure, with interp containing error message.
 */
Tclh_ReturnCode Tclh_PointerUnwrapTagged(Tcl_Interp *interp,
                                         Tclh_PointerRegistry registryP,
                                         Tcl_Obj *objP,
                                         void **pointerP,
                                         Tclh_PointerTypeTag expected_tag);

/* Function: Tclh_PointerUnwrapAnyOf
 * Unwraps a Tcl_Obj representing a pointer checking it is of several
 * possible types. No checks are made with respect to its registration.
 *
 * Parameters:
 * interp   - Interpreter in which to store error messages. May be NULL.
 * registryP - pointer registry. If NULL, the registry associated with the
 *             interpreter is used.
 * objP     - Tcl_Obj holding the wrapped pointer value.
 * pointerP - if not NULL, location to store unwrapped pointer.
 * ...      - List of type tags to match against the pointer. Terminated
 *            by a NULL argument.
 *
 * At least one of interp and registryP must be non-NULL.
 *
 * Returns:
 * TCL_OK    - Success, with the unwrapped pointer stored in *pointerP.
 * TCL_ERROR - Failure, with interp containing error message.
 */
Tclh_ReturnCode Tclh_PointerUnwrapAnyOf(Tcl_Interp *interp,
                                        Tclh_PointerRegistry registryP,
                                        Tcl_Obj *objP,
                                        void **pointerP,
                                        ... /* tag, ..., NULL */);

/* Function: Tclh_PointerEnumerate
 * Returns a list of registered pointers.
 *
 * Parameters:
 * interp - interpreter. May be NULL.
 * registryP - pointer registry. If NULL, the registry associated with the
 *             interpreter is used.
 * tag - Type tag to match. If NULL, pointers with all tags are returned.
 *
 * At least one of interp and registryP must be non-NULL.
 *
 * Returns:
 * Pointer to a Tcl_Obj with reference count 0.
 */
Tcl_Obj *Tclh_PointerEnumerate(Tcl_Interp *interp,
                               Tclh_PointerRegistry registryP,
                               Tclh_PointerTypeTag tag);

/* Function: Tclh_PointerSubtagDefine
 * Registers a tag as a subtype of another tag
 *
 * Parameters:
 * interp - Interpreter. May be NULL.
 * registryP - pointer registry. If NULL, the registry associated with the
 *             interpreter is used.
 * subtagObj - the subtag
 * supertagObj - the super tag
 *
 * The subtag must not have already been registered as a subtag of some
 * other tag.
 *
 * At least one of interp and registryP must be non-NULL.
 *
 * Returns:
 * A Tcl result code.
 */
Tclh_ReturnCode Tclh_PointerSubtagDefine(Tcl_Interp *interp,
                                         Tclh_PointerRegistry registryP,
                                         Tclh_PointerTypeTag subtagObj,
                                         Tclh_PointerTypeTag supertagObj);

/* Function: Tclh_PointerSubtagRemove
 * Remove a subtag definition
 *
 * Parameters:
 * interp - Interpreter. May be NULL.
 * registryP - pointer registry. If NULL, the registry associated with the
 *             interpreter is used.
 * tagObj - the subtag
 *
 * At least one of interp and registryP must be non-NULL.
 *
 * Returns:
 * A Tcl result code.
 */
Tclh_ReturnCode Tclh_PointerSubtagRemove(Tcl_Interp *interp,
                                         Tclh_PointerRegistry registryP,
                                         Tclh_PointerTypeTag tagObj);

/* Function: Tclh_PointerSubtags
 * Returns dictionary mapping subtags to their supertags
 *
 * Parameters:
 * interp - Tcl interpreter. May be NULL.
 * registryP - pointer registry. If NULL, the registry associated with the
 *             interpreter is used.
 *
 * At least one of interp and registryP must be non-NULL.
 *
 * Returns:
 * Dictionary of subtags or NULL on failure.
 */
Tcl_Obj *Tclh_PointerSubtags(Tcl_Interp *interp, Tclh_PointerRegistry registryP);

/* Function: Tclh_PointerCast
 * Changes the tag associated with a pointer
 *
 * Parameters:
 * interp   - Interpreter in which to store error messages. May be NULL.
 * registryP - pointer registry. If NULL, the registry associated with the
 *             interpreter is used.
 * ptrObjP  - Tcl_Obj holding the wrapped pointer value.
 * newTagObj - new tag to which pointer is to be cast. May be NULL to cast
 *   to void pointer
 * castPtrObj - location to store wrapped pointer after casting
 *
 * If the passed wrapped pointer is NULL, a wrapped NULL pointer is returned.
 * If it is a safe pointer, it's tag is changed to the new tag in the pointer
 * registry.
 *
 * For the cast to succeed either newTagObj must be NULL indicating cast
 * to void or the current tag must be derived from it via previous calls
 * to Tclh_PointerSubtagDefine.
 *
 * Returns:
 * A Tcl return code.
 */
Tclh_ReturnCode Tclh_PointerCast(Tcl_Interp *interp,
                                 Tclh_PointerRegistry registryP,
                                 Tcl_Obj *ptrObj,
                                 Tclh_PointerTypeTag newTagObj,
                                 Tcl_Obj **castPtrObj);

#ifdef TCLH_SHORTNAMES

#define PointerLibInit            Tclh_PointerLibInit
#define PointerLibFinit           Tclh_PointerLibFinit
#define PointerRegister           Tclh_PointerRegister
#define PointerUnregister         Tclh_PointerUnregister
#define PointerVerify             Tclh_PointerVerify
#define PointerObjUnregister      Tclh_PointerObjUnregister
#define PointerObjUnregisterAnyOf Tclh_PointerObjUnregisterAnyOf
#define PointerObjVerify          Tclh_PointerObjVerify
#define PointerObjVerifyAnyOf     Tclh_PointerObjVerifyAnyOf
#define PointerWrap               Tclh_PointerWrap
#define PointerUnwrap             Tclh_PointerUnwrap
#define PointerObjGetTag          Tclh_PointerGetTag
#define PointerUnwrapAnyOf        Tclh_PointerUnwrapAnyOf
#define PointerEnumerate          Tclh_PointerEnumerate
#define PointerSubtagDefine       Tclh_PointerSubtagDefine
#define PointerSubtagRemove       Tclh_PointerSubtagRemove
#define PointerSubtags            Tclh_PointerSubtags
#define PointerCast               Tclh_PointerCast

#endif

/*
 * IMPLEMENTATION
 */

#ifdef TCLH_IMPL
# define TCLH_POINTER_IMPL
#endif
#ifdef TCLH_POINTER_IMPL


/*
 * TclhPointerRecord keeps track of pointers and the count of references
 * to them. Pointers that are single reference have a nRefs of -1.
 */
typedef struct TclhPointerRecord {
    Tcl_Obj *tagObj;            /* Identifies the "type". May be NULL */
    int nRefs;                  /* Number of references to the pointer */
} TclhPointerRecord;

typedef struct TclhPointerRegistryInfo {
    Tcl_HashTable pointers;/* Table of registered pointers */
    Tcl_HashTable castables;/* Table of permitted casts subclass -> class */
} TclhPointerRegistryInfo;


/*
 * Pointer is a Tcl "type" whose internal representation is stored
 * as the pointer value and an associated C pointer/handle type.
 * The Tcl_Obj.internalRep.twoPtrValue.ptr1 holds the C pointer value
 * and Tcl_Obj.internalRep.twoPtrValue.ptr2 holds a Tcl_Obj describing
 * the type. This may be NULL if no type info needs to be associated
 * with the value.
 */
static void DupPointerType(Tcl_Obj *srcP, Tcl_Obj *dstP);
static void FreePointerType(Tcl_Obj *objP);
static void UpdatePointerTypeString(Tcl_Obj *objP);
static int  SetPointerFromAny(Tcl_Interp *interp, Tcl_Obj *objP);

static struct Tcl_ObjType gPointerType = {
    "Pointer",
    FreePointerType,
    DupPointerType,
    UpdatePointerTypeString,
    NULL,
};
TCLH_INLINE void *PointerValueGet(Tcl_Obj *objP) {
    return objP->internalRep.twoPtrValue.ptr1;
}
TCLH_INLINE void PointerValueSet(Tcl_Obj *objP, void *valueP) {
    objP->internalRep.twoPtrValue.ptr1 = valueP;
}
/* May return NULL */
TCLH_INLINE Tclh_PointerTypeTag PointerTypeGet(Tcl_Obj *objP) {
    return objP->internalRep.twoPtrValue.ptr2;
}
TCLH_INLINE void PointerTypeSet(Tcl_Obj *objP, Tclh_PointerTypeTag tag) {
    objP->internalRep.twoPtrValue.ptr2 = (void*)tag;
}
static int PointerTypeSame(Tclh_PointerTypeTag pointer_tag,
                           Tclh_PointerTypeTag expected_tag);
static TclhPointerRegistryInfo *TclhInitPointerRegistry(Tcl_Interp *interp);
static int PointerTypeCompatible(TclhPointerRegistryInfo *registryP,
                                 Tclh_PointerTypeTag tag,
                                 Tclh_PointerTypeTag expected);

Tclh_ReturnCode
Tclh_PointerLibInit(Tcl_Interp *interp, Tclh_PointerRegistry *ptrRegP)
{
    int ret;
    ret = Tclh_BaseLibInit(interp);
    if (ret == TCL_OK) {
        Tclh_PointerRegistry reg;
        reg = TclhInitPointerRegistry(interp);
        if (reg) {
            if (ptrRegP)
                *ptrRegP = reg;
        }
        else
            ret = TCL_ERROR;
    }
    return ret;
}

static int
PointerTypeSame(Tclh_PointerTypeTag pointer_tag,
                Tclh_PointerTypeTag expected_tag)
{
    if (pointer_tag == expected_tag || expected_tag == NULL)
        return 1;

    if (pointer_tag == NULL)
        return 0;

    return !strcmp(Tcl_GetString(pointer_tag), Tcl_GetString(expected_tag));
}

Tcl_Obj *
Tclh_PointerWrap(void *pointerValue, Tclh_PointerTypeTag tag)
{
    Tcl_Obj *objP;

    objP = Tcl_NewObj();
    Tcl_InvalidateStringRep(objP);
    PointerValueSet(objP, pointerValue);
    if (tag)
        Tcl_IncrRefCount(tag);
    PointerTypeSet(objP, tag);
    objP->typePtr = &gPointerType;
    return objP;
}

Tclh_ReturnCode
Tclh_PointerUnwrap(Tcl_Interp *interp, Tcl_Obj *objP, void **pvP)
{
    /* Try converting Tcl_Obj internal rep */
    if (objP->typePtr != &gPointerType) {
        if (SetPointerFromAny(interp, objP) != TCL_OK)
            return TCL_ERROR;
    }

    *pvP = PointerValueGet(objP);
    return TCL_OK;
}

Tclh_ReturnCode
Tclh_PointerUnwrapTagged(Tcl_Interp *interp,
                         Tclh_PointerRegistry registryP,
                         Tcl_Obj *objP,
                         void **pvP,
                         Tclh_PointerTypeTag expectedTag)
{
    Tclh_PointerTypeTag tag;
    void *pv;

    /* Try converting Tcl_Obj internal rep */
    if (objP->typePtr != &gPointerType) {
        if (SetPointerFromAny(interp, objP) != TCL_OK)
            return TCL_ERROR;
    }

    tag = PointerTypeGet(objP);
    pv  = PointerValueGet(objP);

    /*
    * Check tag if 
    *   - expectedTag is not NULL,
    *     and
    *   - the unwrapped pointer is not NULL or its tag is not NULL
    */
    if (expectedTag && (pv || tag)) {
        if (registryP == NULL) {
            if (interp == NULL) {
                return TCL_ERROR; /* Either interp or registryP should have been input */
            }
            registryP = TclhInitPointerRegistry(interp);
        }
        if (!PointerTypeCompatible(registryP, tag, expectedTag)) {
            return Tclh_ErrorWrongType(interp, objP, "Pointer type mismatch.");
        }
    }

    *pvP = PointerValueGet(objP);
    return TCL_OK;
}

Tclh_ReturnCode
Tclh_PointerObjGetTag(Tcl_Interp *interp,
                      Tcl_Obj *objP,
                      Tclh_PointerTypeTag *tagPtr)
{
    /* Try converting Tcl_Obj internal rep */
    if (objP->typePtr != &gPointerType) {
        if (SetPointerFromAny(interp, objP) != TCL_OK)
            return TCL_ERROR;
    }
    *tagPtr = PointerTypeGet(objP);
    return TCL_OK;
}

static Tclh_ReturnCode
TclhUnwrapAnyOfVA(Tcl_Interp *interp,
                  Tclh_PointerRegistry registryP,
                  Tcl_Obj *objP,
                  void **pvP,
                  Tclh_PointerTypeTag *tagP,
                  va_list args)
{
    Tclh_PointerTypeTag tag;

    if (registryP == NULL) {
        if (interp == NULL) {
            return TCL_ERROR; /* Either interp or registryP should have been
                                 input */
        }
        registryP = TclhInitPointerRegistry(interp);
    }

    while ((tag = va_arg(args, Tclh_PointerTypeTag)) != NULL) {
        /* Pass in registryP, not interp to avoid interp error message */
        if (Tclh_PointerUnwrapTagged(NULL, registryP, objP, pvP, tag) == TCL_OK) {
            if (tagP)
                *tagP = tag;
            return TCL_OK;
        }
    }

    return Tclh_ErrorWrongType(interp, objP, "Pointer type mismatch.");
}

Tclh_ReturnCode
Tclh_PointerUnwrapAnyOf(Tcl_Interp *interp,
                        Tclh_PointerRegistry registryP,
                        Tcl_Obj *objP,
                        void **pvP,
                        ...)
{
    int     tclResult;
    va_list args;

    va_start(args, pvP);
    tclResult = TclhUnwrapAnyOfVA(interp, registryP, objP, pvP, NULL, args);
    va_end(args);
    return tclResult;
}

static void
UpdatePointerTypeString(Tcl_Obj *objP)
{
    Tclh_PointerTypeTag tagObj;
    Tcl_Size len;
    Tcl_Size taglen;
    char *tagStr;
    char *bytes;

    TCLH_ASSERT(objP->bytes == NULL);
    TCLH_ASSERT(objP->typePtr == &gPointerType);

    tagObj = PointerTypeGet(objP);
    if (tagObj) {
        tagStr = Tcl_GetStringFromObj(tagObj, &taglen);
    }
    else {
        tagStr = "";
        taglen = 0;
    }
    /* Assume 40 bytes enough for address */
    bytes = Tcl_Alloc(40 + 1 + taglen + 1);
    (void) TclhPrintAddress(PointerValueGet(objP), bytes, 40);
    len = Tclh_strlen(bytes);
    bytes[len] = '^';
    memcpy(bytes + len + 1, tagStr, taglen+1);
    objP->bytes = bytes;
    objP->length = len + 1 + taglen;
}

static void
FreePointerType(Tcl_Obj *objP)
{
    Tclh_PointerTypeTag tag = PointerTypeGet(objP);
    if (tag)
        Tcl_DecrRefCount(tag);
    PointerTypeSet(objP, NULL);
    PointerValueSet(objP, NULL);
    objP->typePtr = NULL;
}

static void
DupPointerType(Tcl_Obj *srcP, Tcl_Obj *dstP)
{
    Tclh_PointerTypeTag tag;
    dstP->typePtr = &gPointerType;
    PointerValueSet(dstP, PointerValueGet(srcP));
    tag = PointerTypeGet(srcP);
    if (tag)
        Tcl_IncrRefCount(tag);
    PointerTypeSet(dstP, tag);
}

static int
SetPointerFromAny(Tcl_Interp *interp, Tcl_Obj *objP)
{
    void *pv;
    Tclh_PointerTypeTag tagObj;
    char *srep;
    char *s;

    if (objP->typePtr == &gPointerType)
        return TCL_OK;

    /* Pointers are address^tag, 0 or NULL*/
    srep = Tcl_GetString(objP);
    if (sscanf(srep, "%p^", &pv) == 1) {
        s = strchr(srep, '^');
        if (s == NULL)
            goto invalid_value;
        if (s[1] == '\0')
            tagObj = NULL;
        else {
            tagObj = Tcl_NewStringObj(s + 1, -1);
            Tcl_IncrRefCount(tagObj);
        }
    }
    else {
        if (strcmp(srep, "NULL"))
            goto invalid_value;
        pv = NULL;
        tagObj = NULL;
    }

    /* OK, valid opaque rep. Convert the passed object's internal rep */
    if (objP->typePtr && objP->typePtr->freeIntRepProc) {
        objP->typePtr->freeIntRepProc(objP);
    }
    objP->typePtr = &gPointerType;
    PointerValueSet(objP, pv);
    PointerTypeSet(objP, tagObj);

    return TCL_OK;

invalid_value: /* s must point to value */
    return Tclh_ErrorInvalidValue(interp, objP, "Invalid pointer format.");
}

/*
 * Pointer registry implementation.
 */

static int
PointerTypeError(Tcl_Interp *interp,
                 Tclh_PointerTypeTag registered,
                 Tclh_PointerTypeTag tag)
{
    return Tclh_ErrorWrongType(
        interp, NULL, "Pointer tag does not match registered tag.");
}

static int
PointerNotRegisteredError(Tcl_Interp *interp,
                          const void *p,
                          Tclh_PointerTypeTag tag)
{
    char addr[40];
    char buf[100];
    TclhPrintAddress(p, addr, sizeof(addr));
    snprintf(buf,
             sizeof(buf),
             "Pointer %s^%s is not registered.",
             addr,
             tag ? Tcl_GetString(tag) : "");

    return Tclh_ErrorGeneric(interp, "NOT_FOUND", buf);
}

static void
TclhPointerRecordFree(TclhPointerRecord *ptrRecP)
{
    /* TBD - this assumes pointer tags are tagObj */
    if (ptrRecP->tagObj)
        Tcl_DecrRefCount(ptrRecP->tagObj);
    Tcl_Free((void *)ptrRecP);
}

static void
TclhCleanupPointerRegistry(ClientData clientData, Tcl_Interp *interp)
{
    TclhPointerRegistryInfo *registryP = (TclhPointerRegistryInfo *)clientData;
    Tcl_HashTable *hTblPtr;
    Tcl_HashEntry *he;
    Tcl_HashSearch hSearch;
    hTblPtr = &registryP->pointers;
    for (he = Tcl_FirstHashEntry(hTblPtr, &hSearch); he != NULL;
         he = Tcl_NextHashEntry(&hSearch)) {
        TclhPointerRecordFree((TclhPointerRecord *)Tcl_GetHashValue(he));
    }
    Tcl_DeleteHashTable(&registryP->pointers);

    hTblPtr = &registryP->castables;
    for (he = Tcl_FirstHashEntry(hTblPtr, &hSearch); he != NULL;
         he = Tcl_NextHashEntry(&hSearch)) {
        Tcl_Obj *superTagObj = (Tcl_Obj *)Tcl_GetHashValue(he);
        /* Future proof in case we allow superTagObj as NULL */
        if (superTagObj)
            Tcl_DecrRefCount(superTagObj);
    }
    Tcl_DeleteHashTable(&registryP->castables);

    Tcl_Free((void *)registryP);
}

#ifndef TCLH_POINTER_REGISTRY_NAME
/* This will be shared for all extensions if embedder has not defined it */
# define TCLH_POINTER_REGISTRY_NAME "TclhPointerTable"
#endif

static TclhPointerRegistryInfo *
TclhInitPointerRegistry(Tcl_Interp *interp)
{
    TclhPointerRegistryInfo *registryP;
    const char * const pointerTableKey = TCLH_POINTER_REGISTRY_NAME;
    registryP = Tcl_GetAssocData(interp, pointerTableKey, NULL);
    if (registryP == NULL) {
        registryP = (TclhPointerRegistryInfo *) Tcl_Alloc(sizeof(*registryP));
        Tcl_InitHashTable(&registryP->pointers, TCL_ONE_WORD_KEYS);
        Tcl_InitHashTable(&registryP->castables, TCL_STRING_KEYS);
        Tcl_SetAssocData(
            interp, pointerTableKey, TclhCleanupPointerRegistry, registryP);
    }
    return registryP;
}

Tclh_ReturnCode
TclhPointerRegister(Tcl_Interp *interp,
                    Tclh_PointerRegistry registryP,
                    void *pointer,
                    Tclh_PointerTypeTag tag,
                    Tcl_Obj **objPP,
                    int counted)
{
    Tcl_HashTable *hTblPtr;
    Tcl_HashEntry *he;
    int            newEntry;
    TclhPointerRecord *ptrRecP;

    if (pointer == NULL)
        return Tclh_ErrorInvalidValue(interp, NULL, "Attempt to register null pointer.");

    if (registryP == NULL) {
        if (interp == NULL)
            return TCL_ERROR;
        registryP = TclhInitPointerRegistry(interp);
    }
    hTblPtr   = &registryP->pointers;
    he = Tcl_CreateHashEntry(hTblPtr, pointer, &newEntry);

    if (he) {
        if (newEntry) {
            ptrRecP = (TclhPointerRecord *)Tcl_Alloc(sizeof(*ptrRecP));
            if (tag) {
                Tcl_IncrRefCount(tag);
                ptrRecP->tagObj = tag;
            }
            else
                ptrRecP->tagObj = NULL;
            /* -1 => uncounted pointer (only single reference allowed) */
            ptrRecP->nRefs = counted ? 1 : -1;
            Tcl_SetHashValue(he, ptrRecP);
        } else {
            ptrRecP = Tcl_GetHashValue(he);
            /*
             * If already existing, existing and passed-in pointer must
             * - both must have the same type tag
             * - both be counted or both single reference, and
             */
            if (!PointerTypeSame(ptrRecP->tagObj, tag))
                return PointerTypeError(interp, ptrRecP->tagObj, tag);
            if (counted) {
                if (ptrRecP->nRefs < 0)
                    return Tclh_ErrorExists(
                        interp,
                        "Registered uncounted pointer",
                        NULL,
                        "Attempt to register a counted pointer.");
                ptrRecP->nRefs += 1;
            }
            else {
                if (ptrRecP->nRefs >= 0)
                    return Tclh_ErrorExists(
                        interp,
                        "Registered counted pointer",
                        NULL,
                        "Attempt to register an uncounted pointer.");
                /* Note ref count NOT incremented - not a counted pointer */
            }
        }
        if (objPP)
            *objPP = Tclh_PointerWrap(pointer, tag);
    } else {
        TCLH_PANIC("Failed to allocate hash table entry.");
        return TCL_ERROR; /* NOT REACHED but keep compiler happy */
    }
    return TCL_OK;
}

Tclh_ReturnCode
Tclh_PointerRegister(Tcl_Interp *interp,
                     Tclh_PointerRegistry registryP,
                     void *pointer,
                     Tclh_PointerTypeTag tag,
                     Tcl_Obj **objPP)
{
    return TclhPointerRegister(interp, registryP, pointer, tag, objPP, 0);
}

Tclh_ReturnCode
Tclh_PointerRegisterCounted(Tcl_Interp *interp,
                            Tclh_PointerRegistry registryP,
                            void *pointer,
                            Tclh_PointerTypeTag tag,
                            Tcl_Obj **objPP)
{
    return TclhPointerRegister(interp, registryP, pointer, tag, objPP, 1);
}

static int
PointerTypeCompatible(TclhPointerRegistryInfo *registryP,
                      Tclh_PointerTypeTag tag,
                      Tclh_PointerTypeTag expected)
{
    int i;

    /* Rather than trying to detect loops by recording history
     * just keep a hard limit 10 on the depth of lookups
     */
    /* NOTE: on first go around ok for tag to be NULL. */
    if (PointerTypeSame(tag, expected))
        return 1;
    /* For NULL, if first did not match succeeding will not either  */
    if (tag == NULL)
        return 0;
    for (i = 0; i < 10; ++i) {
        Tcl_HashEntry *he;
        he = Tcl_FindHashEntry(&registryP->castables, Tcl_GetString(tag));
        if (he == NULL)
            return 0;/* No super type */
        tag = Tcl_GetHashValue(he);
        /* For NULL, if first did not match succeeding will not either cut short */
        if (tag == NULL)
            return 0;
        if (PointerTypeSame(tag, expected))
            return 1;
    }

    return 0;
}

static Tclh_ReturnCode
PointerVerifyOrUnregister(Tcl_Interp *interp,
                          Tclh_PointerRegistry registryP,
                          const void *pointer,
                          Tclh_PointerTypeTag tag,
                          int unregister)
{
    Tcl_HashEntry *he;

    if (registryP == NULL) {
        if (interp == NULL)
            return TCL_ERROR;
        registryP = TclhInitPointerRegistry(interp);
    }
    he = Tcl_FindHashEntry(&registryP->pointers, pointer);
    if (he) {
        TclhPointerRecord *ptrRecP = Tcl_GetHashValue(he);
        if (!PointerTypeCompatible(registryP, ptrRecP->tagObj, tag)) {
            return PointerTypeError(interp, ptrRecP->tagObj, tag);
        }
        if (unregister) {
            if (ptrRecP->nRefs <= 1) {
                /*  Either uncounted or ref count will reach 0 */
                TclhPointerRecordFree(ptrRecP);
                Tcl_DeleteHashEntry(he);
            }
            else
                ptrRecP->nRefs -= 1;
        }
        return TCL_OK;
    }
    return PointerNotRegisteredError(interp, pointer, tag);
}

Tclh_ReturnCode
Tclh_PointerUnregister(Tcl_Interp *interp,
                       Tclh_PointerRegistry registryP,
                       const void *pointer,
                       Tclh_PointerTypeTag tag)
{
    return PointerVerifyOrUnregister(interp, registryP, pointer, tag, 1);
}

Tcl_Obj *
Tclh_PointerEnumerate(Tcl_Interp *interp,
                      Tclh_PointerRegistry registryP,
                      Tclh_PointerTypeTag tag)
{
    Tcl_HashEntry *he;
    Tcl_HashSearch hSearch;
    Tcl_HashTable *hTblPtr;
    Tcl_Obj *resultObj = Tcl_NewListObj(0, NULL);

    if (registryP == NULL) {
        if (interp == NULL)
            return resultObj;
        registryP = TclhInitPointerRegistry(interp);
    }
    hTblPtr   = &registryP->pointers;
    for (he = Tcl_FirstHashEntry(hTblPtr, &hSearch);
         he != NULL; he = Tcl_NextHashEntry(&hSearch)) {
        void *pv                   = Tcl_GetHashKey(hTblPtr, he);
        TclhPointerRecord *ptrRecP = Tcl_GetHashValue(he);
        if (PointerTypeSame(ptrRecP->tagObj, tag)) {
            Tcl_ListObjAppendElement(NULL, resultObj, Tclh_PointerWrap(pv, ptrRecP->tagObj));
        }
    }
    return resultObj;
}

Tclh_ReturnCode
Tclh_PointerVerify(Tcl_Interp *interp,
                   Tclh_PointerRegistry registryP,
                   const void *pointer,
                   Tclh_PointerTypeTag tag)
{
    return PointerVerifyOrUnregister(interp, registryP, pointer, tag, 0);
}

Tclh_ReturnCode
Tclh_PointerObjUnregister(Tcl_Interp *interp,
                          Tclh_PointerRegistry registryP,
                          Tcl_Obj *objP,
                          void **pointerP,
                          Tclh_PointerTypeTag tag)
{
    void *pv = NULL;            /* Init to keep gcc happy */
    int   tclResult;

    tclResult = Tclh_PointerUnwrapTagged(interp, registryP, objP, &pv, tag);
    if (tclResult == TCL_OK) {
        if (pv != NULL)
            tclResult = Tclh_PointerUnregister(interp, registryP, pv, tag);
        if (tclResult == TCL_OK && pointerP != NULL)
            *pointerP = pv;
    }
    return tclResult;
}

static Tclh_ReturnCode
PointerObjVerifyOrUnregisterAnyOf(Tcl_Interp *interp,
                                  Tclh_PointerRegistry registryP,
                                  Tcl_Obj *objP,
                                  void **pointerP,
                                  int unregister,
                                  va_list args)
{
    int tclResult;
    void *pv = NULL;            /* Init to keep gcc happy */
    Tclh_PointerTypeTag tag = NULL;

    tclResult = TclhUnwrapAnyOfVA(interp, registryP, objP, &pv, &tag, args);
    if (tclResult == TCL_OK) {
        if (unregister)
            tclResult = Tclh_PointerUnregister(interp, registryP, pv, tag);
        else
            tclResult = Tclh_PointerVerify(interp, registryP, pv, tag);
        if (tclResult == TCL_OK && pointerP != NULL)
            *pointerP = pv;
    }
    return tclResult;
}

Tclh_ReturnCode
Tclh_PointerObjUnregisterAnyOf(Tcl_Interp *interp,
                               Tclh_PointerRegistry registryP,
                               Tcl_Obj *objP,
                               void **pointerP,
                               ... /* tag, ... , NULL */)
{
    int tclResult;
    va_list args;

    va_start(args, pointerP);
    tclResult = PointerObjVerifyOrUnregisterAnyOf(interp, registryP, objP, pointerP, 1, args);
    va_end(args);
    return tclResult;
}

Tclh_ReturnCode
Tclh_PointerObjVerify(Tcl_Interp *interp,
                      Tclh_PointerRegistry registryP,
                      Tcl_Obj *objP,
                      void **pointerP,
                      Tclh_PointerTypeTag tag)
{
    void *pv = NULL;            /* Init to keep gcc happy */
    int   tclResult;

    tclResult = Tclh_PointerUnwrapTagged(interp, registryP, objP, &pv, tag);
    if (tclResult == TCL_OK) {
        if (pv == NULL)
            tclResult = Tclh_ErrorInvalidValue(interp, NULL, "Pointer is NULL.");
        else {
            tclResult = Tclh_PointerVerify(interp, registryP, pv, tag);
            if (tclResult == TCL_OK) {
                if (pointerP)
                    *pointerP = pv;
            }
        }
    }
    return tclResult;
}

Tclh_ReturnCode
Tclh_PointerObjVerifyAnyOf(Tcl_Interp *interp,
                           Tclh_PointerRegistry registryP,
                           Tcl_Obj *objP,
                           void **pointerP,
                           ... /* tag, tag, NULL */)
{
    int tclResult;
    va_list args;

    va_start(args, pointerP);
    tclResult = PointerObjVerifyOrUnregisterAnyOf(interp, registryP, objP, pointerP, 0, args);
    va_end(args);
    return tclResult;
}

Tclh_ReturnCode
Tclh_PointerSubtagDefine(Tcl_Interp *interp,
                         Tclh_PointerRegistry registryP,
                         Tclh_PointerTypeTag subtagObj,
                         Tclh_PointerTypeTag supertagObj)
{
    int tclResult;
    const char *subtag;

    if (registryP == NULL) {
        if (interp == NULL)
            return TCL_ERROR;
        registryP = TclhInitPointerRegistry(interp);
    }

    if (supertagObj == NULL)
        return TCL_OK; /* void* always a supertype, no need to register */
    subtag = Tcl_GetString(subtagObj);
    if (!strcmp(subtag, Tcl_GetString(supertagObj)))
        return TCL_OK; /* Same tag */
    tclResult = Tclh_HashAdd(
        interp, &registryP->castables, subtag, supertagObj);
    if (tclResult == TCL_OK)
        Tcl_IncrRefCount(supertagObj);/* Since added to hash table */
    return tclResult;
}

Tclh_ReturnCode
Tclh_PointerSubtagRemove(Tcl_Interp *interp,
                         Tclh_PointerRegistry registryP,
                         Tclh_PointerTypeTag tagObj)
{
    Tcl_HashEntry *he;

    if (registryP == NULL) {
        if (interp == NULL)
            return TCL_ERROR;
        registryP = TclhInitPointerRegistry(interp);
    }

    if (tagObj) {
        he = Tcl_FindHashEntry(&registryP->castables, Tcl_GetString(tagObj));
        if (he) {
            Tcl_Obj *objP = Tcl_GetHashValue(he);
            if (objP)
                Tcl_DecrRefCount(objP);
            Tcl_DeleteHashEntry(he);
        }
    }
    return TCL_OK;
}

Tcl_Obj *
Tclh_PointerSubtags(Tcl_Interp *interp, Tclh_PointerRegistry registryP)
{
    Tcl_Obj *objP;
    Tcl_HashEntry *heP;
    Tcl_HashTable *htP;
    Tcl_HashSearch hSearch;

    objP = Tcl_NewListObj(0, NULL);
    if (registryP == NULL) {
        if (interp == NULL)
            return objP;
        registryP = TclhInitPointerRegistry(interp);
    }
    htP  = &registryP->castables;

    for (heP = Tcl_FirstHashEntry(htP, &hSearch); heP != NULL;
         heP = Tcl_NextHashEntry(&hSearch)) {
        Tcl_Obj *subtagObj = Tcl_NewStringObj(Tcl_GetHashKey(htP, heP), -1);
        Tcl_Obj *supertagObj = Tcl_GetHashValue(heP);
        if (supertagObj == NULL)
            supertagObj = Tcl_NewObj();
        Tcl_ListObjAppendElement(NULL, objP, subtagObj);
        Tcl_ListObjAppendElement(NULL, objP, supertagObj);
    }

    return objP;
}

Tclh_ReturnCode
Tclh_PointerCast(Tcl_Interp *interp,
                 Tclh_PointerRegistry registryP,
                 Tcl_Obj *objP,
                 Tclh_PointerTypeTag newTag,
                 Tcl_Obj **castPtrObj)
{
    Tclh_PointerTypeTag oldTag;
    void *pv;
    int tclResult;

    /* First extract pointer value and tag */
    tclResult = Tclh_PointerObjGetTag(interp, objP, &oldTag);
    if (tclResult != TCL_OK)
        return tclResult;

    tclResult = Tclh_PointerUnwrapTagged(interp, registryP, objP, &pv, NULL);
    if (tclResult != TCL_OK)
        return tclResult;

    /* Validate that if registered, it is registered correctly. */
    if (registryP == NULL && interp != NULL) {
        registryP = TclhInitPointerRegistry(interp);
    }
    /*
     * Note it is NOT an error if registry and interp are both NULL.
     * It simply means it is not a registered pointer and the registration
     * does not need to be updated.
     */
    if (registryP) {
        TclhPointerRecord *ptrRecP = NULL;
        Tcl_HashEntry *he = Tcl_FindHashEntry(&registryP->pointers, pv);
        if (he) {
            ptrRecP = Tcl_GetHashValue(he);
            if (!PointerTypeSame(oldTag, ptrRecP->tagObj)) {
                /* Pointer is registered but as a different type */
                return PointerTypeError(interp, ptrRecP->tagObj, oldTag);
            }
        }
        /* Must be either upcastable or downcastable */
        if (!PointerTypeCompatible(registryP, oldTag, newTag)
            && !PointerTypeCompatible(registryP, newTag, oldTag)) {
            return Tclh_ErrorGeneric(
                interp,
                "POINTER",
                "Pointer tags are not compatible for casting.");
        }
        /* If registered, we have to change registration */
        if (ptrRecP) {
            Tclh_PointerTypeTag tempTag;
            tempTag         = ptrRecP->tagObj;
            ptrRecP->tagObj = newTag;
            if (newTag)
                Tcl_IncrRefCount(newTag);
            if (tempTag)
                Tcl_DecrRefCount(tempTag); /* AFTER incr-ing newTag */
        }
    }

    *castPtrObj = Tclh_PointerWrap(pv, newTag);
    return TCL_OK;
}

#endif /* TCLH_POINTER_IMPL */

#endif /* TCLHPOINTER_H */
