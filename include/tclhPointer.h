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
#include "tclhAtom.h"
#include <stdarg.h>

/* Typedef: Tclh_PointerTypeTag
 *
 * See <Pointer Type Tags>.
 */
typedef Tcl_Obj *Tclh_PointerTypeTag;

/* Typedef: Tclh_PointerTagRelation
 */
typedef enum Tclh_PointerTagRelation {
    TCLH_TAG_RELATION_UNRELATED, /* Tags are unrelated and not convertible */
    TCLH_TAG_RELATION_EQUAL,     /* Tags are the same */
    TCLH_TAG_RELATION_IMPLICITLY_CASTABLE, /* Tag is implicitly castable */
    TCLH_TAG_RELATION_EXPLICITLY_CASTABLE, /* Tag is explicitly castable */
} Tclh_PointerTagRelation;

/* Typedef: Tclh_PointerRegistrationStatus
 */
typedef enum Tclh_PointerRegistrationStatus {
    TCLH_POINTER_REGISTRATION_MISSING,  /* Pointer is not registered */
    TCLH_POINTER_REGISTRATION_WRONGTAG, /* Pointer is registered with different
                                           tag */
    TCLH_POINTER_REGISTRATION_OK,       /* Pointer tag matches registration */
    TCLH_POINTER_REGISTRATION_DERIVED,  /* Pointer tag is implicitly castable to
                                           registration */
} Tclh_PointerRegistrationStatus;

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
 * checked with <Tclh_PointerVerifyTagged>. Pointers should be marked invalid as
 * appropriate by unregistering them with <Tclh_PointerUnregisterTagged> or
 * alternatively <Tclh_PointerObjUnregister>. For convenience, when a pointer
 * may match one of several types, the <Tclh_PointerVerifyAnyOf> and
 * <Tclh_PointerObjVerifyAnyOf> take a variable number of type tags.
 *
 * If pointer registration is not deemed necessary (dangerous), the
 * functions <Tclh_PointerWrap> and <Tclh_PointerUnwrapTagged> can be used
 * to convert pointers to and from Tcl_Obj values.
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
 * interp - Tcl interpreter for error messages. May be NULL.
 * tclhCtxP - Tclh context as returned by <Tclh_LibInit> to use. If NULL,
 *    the Tclh context associated with the interpreter is used after
 *    initialization if necessary.
 *
 * At least one of interp and tclhCtxP must be non-NULL.
 *
 * Any allocated resource are automatically freed up when the interpreter
 * is deleted.
 *
 * Returns:
 * TCL_OK    - Library was successfully initialized.
 * TCL_ERROR - Initialization failed. Library functions must not be called.
 *             An error message is left in the interpreter result.
 */
TCLH_LOCAL Tclh_ReturnCode Tclh_PointerLibInit(Tcl_Interp *interp,
                                    Tclh_LibContext *tclhCtxP);

/* Function: Tclh_PointerRegister
 * Registers a pointer value as being "valid".
 *
 * Parameters:
 * interp  - Tcl interpreter in which the pointer is to be registered.
 *           May be NULL.
 * tclhCtxP - Tclh context as returned by <Tclh_LibInit> to use. If NULL,
 *            the Tclh context associated with the interpreter is used.
 * pointer - Pointer value to be registered.
 * tag     - Type tag for the pointer. Pass NULL or 0 for typeless pointers.
 * objPP   - if not NULL, a pointer to a new Tcl_Obj holding the pointer
 *           representation is stored here on success. The Tcl_Obj has
 *           a reference count of 0.
 *
 * At least one of interp and tclhCtxP must be non-NULL.
 *
 * The validity of a registered pointer can then be tested by with
 * <Tclh_PointerVerifyTagged> and reversed by calling <Tclh_PointerUnregisterTagged>.
 * Registering a pointer that is already registered will raise an error if
 * the tags do not match or if the previous registration was for a counted
 * pointer. Otherwise the duplicate registration is a no-op and the pointer
 * is unregistered on the next call to <Tclh_PointerUnregisterTagged> no matter
 * how many times it has been registered.
 *
 * Returns:
 * TCL_OK    - pointer was successfully registered
 * TCL_ERROR - pointer registration failed. An error message is stored in
 *             the interpreter.
 */
TCLH_LOCAL Tclh_ReturnCode Tclh_PointerRegister(Tcl_Interp *interp,
                                     Tclh_LibContext *tclhCtxP,
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
 * tclhCtxP - Tclh context as returned by <Tclh_LibInit> to use. If NULL,
 *            the Tclh context associated with the interpreter is used.
 * pointer - Pointer value to be registered.
 * tag     - Type tag for the pointer. Pass NULL or 0 for typeless pointers.
 * objPP   - if not NULL, a pointer to a new Tcl_Obj holding the pointer
 *           representation is stored here on success. The Tcl_Obj has
 *           a reference count of 0.
 *
 * At least one of interp and tclhCtxP must be non-NULL.
 *
 * The validity of a registered pointer can then be tested by with
 * <Tclh_PointerVerifyTagged> and reversed by calling <Tclh_PointerUnregisterTagged>.
 * A counted pointer that is registered multiple times will be treated
 * as valid until the same number of calls to <Tclh_PointerUnregisterTagged>.
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
TCLH_LOCAL Tclh_ReturnCode
Tclh_PointerRegisterCounted(Tcl_Interp *interp,
                            Tclh_LibContext *tclhCtxP,
                            void *pointer,
                            Tclh_PointerTypeTag tag,
                            Tcl_Obj **objPP);

/* Function: Tclh_PointerPin
 * Registers a pointer value as pinned so it is always deemed valid
 * and is not affected by unregistrations.
 *
 * Parameters:
 * interp  - Tcl interpreter in which the pointer is to be registered.
 *           May be NULL.
 * tclhCtxP - Tclh context as returned by <Tclh_LibInit> to use. If NULL,
 *            the Tclh context associated with the interpreter is used.
 * pointer - Pointer value to be pinned. This may or may not be already
 *           registered.
 * tag     - Type tag for the pointer. Pass NULL or 0 for typeless pointers.
 *           The returned wrapper will be tagged but the registration will
 *           not be associated with a tag.
 * objPP   - if not NULL, a pointer to a new Tcl_Obj holding the pointer
 *           representation is stored here on success. The Tcl_Obj has
 *           a reference count of 0.
 *
 * At least one of interp and tclhCtxP must be non-NULL.
 *
 * The validity of a registered pointer can then be tested by with
 * <Tclh_PointerVerifyTagged> and reversed by calling <Tclh_PointerUnpin>.
 *
 * Returns:
 * TCL_OK    - pointer was successfully registered
 * TCL_ERROR - pointer registration failed. An error message is stored in
 *             the interpreter.
 */
TCLH_LOCAL Tclh_ReturnCode Tclh_PointerRegisterPinned(Tcl_Interp *interp,
                                                      Tclh_LibContext *tclhCtxP,
                                                      void *pointer,
                                                      Tclh_PointerTypeTag tag,
                                                      Tcl_Obj **objPP);

/* Function: Tclh_PointerUnregister
 * Unregisters a previously registered pointer.
 *
 * Parameters:
 * interp   - Tcl interpreter in which the pointer is to be unregistered.
 *           May be NULL.
 * tclhCtxP - Tclh context as returned by <Tclh_LibInit> to use. If NULL,
 *            the Tclh context associated with the interpreter is used.
 * pointer  - Pointer value to be unregistered.
 *
 * At least one of interp and tclhCtxP must be non-NULL.
 *
 * The pointer may have been registered either via <Tclh_PointerRegister> or
 * <Tclh_PointerRegisterCounted>. In the former case, the pointer becomes
 * immediately inaccessible (as defined by Tclh_PointerVerify). In the latter
 * case, it will become inaccessible if it has been unregistered as many times
 * as it has been registered.
 *
 * Returns:
 * TCL_OK    - The pointer was successfully unregistered.
 * TCL_ERROR - The pointer was not registered. An error message is 
 *             left in interp.
 */
TCLH_LOCAL Tclh_ReturnCode Tclh_PointerUnregister(Tcl_Interp *interp,
                                                  Tclh_LibContext *tclhCtxP,
                                                  const void *pointer);

/* Function: Tclh_PointerUnregisterTagged
 * Unregisters a previously registered pointer.
 *
 * Parameters:
 * interp   - Tcl interpreter in which the pointer is to be unregistered.
 *           May be NULL.
 * tclhCtxP - Tclh context as returned by <Tclh_LibInit> to use. If NULL,
 *            the Tclh context associated with the interpreter is used.
 * pointer  - Pointer value to be unregistered.
 * expected_tag  - Type tag for the pointer.
 *
 * At least one of interp and tclhCtxP must be non-NULL.
 *
 * The pointer may have been registered either via <Tclh_PointerRegister> or
 * <Tclh_PointerRegisterCounted>. In the former case, the pointer becomes
 * immediately inaccessible (as defined by Tclh_PointerVerifyTagged). In the latter
 * case, it will become inaccessible if it has been unregistered as many times
 * as it has been registered.
 *
 * Returns:
 * TCL_OK    - The pointer was successfully unregistered.
 * TCL_ERROR - The pointer was not registered or was registered with a
 *             different type. An error message is left in interp.
 */
TCLH_LOCAL Tclh_ReturnCode
Tclh_PointerUnregisterTagged(Tcl_Interp *interp,
                             Tclh_LibContext *tclhCtxP,
                             const void *pointer,
                             Tclh_PointerTypeTag expected_tag);

/* Function: Tclh_PointerInvalidateTagged
 * Removes a pointer from the pointer registration database.
 *
 * Parameters:
 * interp   - Tcl interpreter in which the pointer is to be unregistered.
 *           May be NULL.
 * tclhCtxP - Tclh context as returned by <Tclh_LibInit> to use. If NULL,
 *            the Tclh context associated with the interpreter is used.
 * pointer  - Pointer value to be unregistered.
 * expected_tag  - Type tag for the pointer.
 *
 * At least one of interp and tclhCtxP must be non-NULL.
 *
 * The pointer may have been registered via <Tclh_PointerRegister>,
 * <Tclh_PointerRegisterCounted> or <Tclh_PointerPin>. In all cases, the
 * pointer becomes invalid (as defined by Tclh_PointerVerifyTagged).
 *
 * Returns:
 * TCL_OK    - The pointer was successfully unregistered or was not
 *             registered in the first place.
 * TCL_ERROR - The pointer was registered with a different type.
 *             An error message is left in interp.
 */
TCLH_LOCAL Tclh_ReturnCode
Tclh_PointerInvalidateTagged(Tcl_Interp *interp,
                       Tclh_LibContext *tclhCtxP,
                       const void *pointer,
                       Tclh_PointerTypeTag expected_tag);

/* Function: Tclh_PointerRegistered
 * Verifies that the passed pointer is registered as a valid pointer.
 *
 * interp   - Tcl interpreter in which the pointer is to be verified.
 *           May be NULL.
 * tclhCtxP - Tclh context as returned by <Tclh_LibInit> to use. If NULL,
 *            the Tclh context associated with the interpreter is used.
 * pointer  - Pointer value to be verified.
 *
 * At least one of interp and tclhCtxP must be non-NULL.
 *
 * Returns:
 * 1    - The pointer is registered.
 * 0    - The pointer is not registered.
 */
TCLH_LOCAL int Tclh_PointerRegistered(Tcl_Interp *interp,
                                                  Tclh_LibContext *tclhCtxP,
                                                  const void *voidP);

/* Function: Tclh_PointerVerify
 * Raises an error if the passed pointer is not registered as a valid pointer.
 *
 * interp   - Tcl interpreter in which the pointer is to be verified.
 *           May be NULL.
 * tclhCtxP - Tclh context as returned by <Tclh_LibInit> to use. If NULL,
 *            the Tclh context associated with the interpreter is used.
 * pointer  - Pointer value to be verified.
 *
 * At least one of interp and tclhCtxP must be non-NULL.
 *
 * Returns:
 * TCL_OK    - The pointer is registered.
 * TCL_ERROR - The pointer is not registered. An error message is left in
 *             the interpreter.
 */
TCLH_LOCAL Tclh_ReturnCode Tclh_PointerVerify(
    Tcl_Interp *interp, Tclh_LibContext *tclhCtxP, const void *voidP);

/* Function: Tclh_PointerVerifyTagged
 * Verifies that the passed pointer is registered as a valid pointer
 * of a given type.
 *
 * interp   - Tcl interpreter in which the pointer is to be verified.
 *           May be NULL.
 * tclhCtxP - Tclh context as returned by <Tclh_LibInit> to use. If NULL,
 *            the Tclh context associated with the interpreter is used.
 * pointer  - Pointer value to be verified.
 * expected_tag - Type tag for the pointer. This must be implicitly castable
 *                to the registered type.
 *
 * At least one of interp and tclhCtxP must be non-NULL.
 *
 * Returns:
 * TCL_OK    - The pointer is registered with the same type tag.
 * TCL_ERROR - Pointer is unregistered or a different type. An error message
 *             is stored in interp.
 */
TCLH_LOCAL Tclh_ReturnCode Tclh_PointerVerifyTagged(Tcl_Interp *interp,
                                   Tclh_LibContext *tclhCtxP,
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
TCLH_LOCAL Tclh_ReturnCode Tclh_PointerObjGetTag(Tcl_Interp *interp,
                                      Tcl_Obj *objP,
                                      Tclh_PointerTypeTag *tagPtr);

/* Function: Tclh_PointerObjUnregister
 * Unregisters a previously registered pointer passed in as a Tcl_Obj.
 * Null pointers are silently ignored without an error being raised.
 *
 * Parameters:
 * interp   - Tcl interpreter in which the pointer is to be unregistered.
 *            May be NULL.
 * tclhCtxP - Tclh context as returned by <Tclh_LibInit> to use. If NULL,
 *            the Tclh context associated with the interpreter is used.
 * objP     - Tcl_Obj containing a pointer value to be unregistered.
 * pointerP - If not NULL, the pointer value from objP is stored here
 *            on success.
 * tagPtr   - Type tag for the pointer. May be 0 or NULL if pointer
 *            type is not to be checked.
 *
 * At least one of interp and tclhCtxP must be non-NULL.
 *
 * Returns:
 * TCL_OK    - The pointer was successfully unregistered.
 * TCL_ERROR - The pointer was not registered or was registered with a
 *             different type. An error message is left in interp.
 */
TCLH_LOCAL Tclh_ReturnCode Tclh_PointerObjUnregister(Tcl_Interp *interp,
                                          Tclh_LibContext *tclhCtxP,
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
 * tclhCtxP - Tclh context as returned by <Tclh_LibInit> to use. If NULL,
 *            the Tclh context associated with the interpreter is used.
 * objP     - Tcl_Obj containing a pointer value to be unregistered.
 * pointerP - If not NULL, the pointer value from objP is stored here
 *            on success.
 * tag      - Type tag for the pointer. May be 0 or NULL if pointer
 *            type is not to be checked.
 *
 * At least one of interp and tclhCtxP must be non-NULL.
 *
 * Returns:
 * TCL_OK    - The pointer was successfully unregistered.
 * TCL_ERROR - The pointer was not registered or was registered with a
 *             different type. An error message is left in interp.
 */
TCLH_LOCAL Tclh_ReturnCode Tclh_PointerObjUnregisterAnyOf(Tcl_Interp *interp,
                                               Tclh_LibContext *tclhCtxP,
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
 * tclhCtxP - Tclh context as returned by <Tclh_LibInit> to use. If NULL,
 *            the Tclh context associated with the interpreter is used.
 * objP     - Tcl_Obj containing a pointer value to be verified.
 * pointerP - If not NULL, the pointer value from objP is stored here
 *            on success.
 * tagP -     If not NULL, the pointer tag is stored here on success.
 *            Note this is not necessarily *expected_tag* but implicitly
 *            convertible to it.
 * expected_tag - Type tag for the pointer. May be *NULL* if type is not
 *                to be checked.
 *
 * At least one of interp and tclhCtxP must be non-NULL.
 *
 * Returns:
 * TCL_OK    - The pointer was successfully verified.
 * TCL_ERROR - The pointer was not registered or was registered with a
 *             different type. An error message is left in interp.
 */
TCLH_LOCAL Tclh_ReturnCode
Tclh_PointerObjVerify(Tcl_Interp *interp,
                      Tclh_LibContext *tclhCtxP,
                      Tcl_Obj *objP,
                      void **pointerP,
                      Tclh_PointerTypeTag *tagP,
                      Tclh_PointerTypeTag expected_tag);

/* Function: Tclh_PointerObjVerifyAnyOf
 * Verifies a Tcl_Obj contains a wrapped pointer that is registered
 * and one of several allowed types.
 *
 * Parameters:
 * interp   - Tcl interpreter in which the pointer is to be verified.
 *            May be NULL.
 * tclhCtxP - Tclh context as returned by <Tclh_LibInit> to use. If NULL,
 *            the Tclh context associated with the interpreter is used.
 * objP     - Tcl_Obj containing a pointer value to be verified.
 * pointerP - If not NULL, the pointer value from objP is stored here
 *            on success.
 * ...      - Remaining arguments are type tags terminated by a NULL argument.
 *            The pointer must be one of these types.
 *
 * At least one of interp and tclhCtxP must be non-NULL.
 *
 * Returns:
 * TCL_OK    - The pointer was successfully verified.
 * TCL_ERROR - The pointer was not registered or was registered with a
 *             type that is not one of the passed ones.
 *             An error message is left in interp.
 */
TCLH_LOCAL Tclh_ReturnCode Tclh_PointerObjVerifyAnyOf(Tcl_Interp *interp,
                                           Tclh_LibContext *tclhCtxP,
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
TCLH_LOCAL Tcl_Obj *Tclh_PointerWrap(void *pointer, Tclh_PointerTypeTag tag);

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
TCLH_LOCAL Tclh_ReturnCode
Tclh_PointerUnwrap(Tcl_Interp *interp, Tcl_Obj *objP, void **pointerP);

/* Function: Tclh_PointerUnwrapTagged
 * Unwraps a Tcl_Obj representing a pointer checking it is of the
 * expected type. No checks are made with respect to its registration.
 *
 * Parameters:
 * interp - Interpreter in which to store error messages. May be NULL.
 * tclhCtxP - Tclh context as returned by <Tclh_LibInit> to use. If NULL,
 *            the Tclh context associated with the interpreter is used.
 * objP   - Tcl_Obj holding the wrapped pointer value.
 * pointerP - if not NULL, location to store unwrapped pointer on success.
 * tagP - if not NULL, the pointer tag is stored here on success. Note
 *            this may not be same as *expected_tag* but one that is
 *            implicitly convertible to it.
 * expected_tag - Type tag for the pointer. May be *NULL* if type is not
 *                to be checked. If not NULL, at least one of interp or
 *                registryP must not be NULL.
 *
 * At least one of interp and tclhCtxP must be non-NULL.
 * 
 * The function does not check tags if the unwrapped pointer is NULL and 
 * is also untagged.
 *
 * Returns:
 * TCL_OK    - Success, with the unwrapped pointer stored in *pointerP.
 * TCL_ERROR - Failure, with interp containing error message.
 */
TCLH_LOCAL Tclh_ReturnCode Tclh_PointerUnwrapTagged(Tcl_Interp *interp,
                                         Tclh_LibContext *tclhCtxP,
                                         Tcl_Obj *objP,
                                         void **pointerP,
                                         Tclh_PointerTypeTag *tagP,
                                         Tclh_PointerTypeTag expected_tag);

/* Function: Tclh_PointerUnwrapAnyOf
 * Unwraps a Tcl_Obj representing a pointer checking it is of several
 * possible types. No checks are made with respect to its registration.
 *
 * Parameters:
 * interp   - Interpreter in which to store error messages. May be NULL.
 * tclhCtxP - Tclh context as returned by <Tclh_LibInit> to use. If NULL,
 *            the Tclh context associated with the interpreter is used.
 * objP     - Tcl_Obj holding the wrapped pointer value.
 * pointerP - if not NULL, location to store unwrapped pointer.
 * ...      - List of type tags to match against the pointer. Terminated
 *            by a NULL argument.
 *
 * At least one of interp and tclhCtxP must be non-NULL.
 *
 * Returns:
 * TCL_OK    - Success, with the unwrapped pointer stored in *pointerP.
 * TCL_ERROR - Failure, with interp containing error message.
 */
TCLH_LOCAL Tclh_ReturnCode Tclh_PointerUnwrapAnyOf(Tcl_Interp *interp,
                                        Tclh_LibContext *tclhCtxP,
                                        Tcl_Obj *objP,
                                        void **pointerP,
                                        ... /* tag, ..., NULL */);

/* Function: Tclh_PointerEnumerate
 * Returns a list of registered pointers.
 *
 * Parameters:
 * interp - interpreter. May be NULL.
 * tclhCtxP - Tclh context as returned by <Tclh_LibInit> to use. If NULL,
 *            the Tclh context associated with the interpreter is used.
 * tag - Type tag to match. If NULL, pointers with all tags are returned.
 *
 * At least one of interp and tclhCtxP must be non-NULL.
 *
 * Returns:
 * Pointer to a Tcl_Obj with reference count 0.
 */
TCLH_LOCAL Tcl_Obj *Tclh_PointerEnumerate(Tcl_Interp *interp,
                               Tclh_LibContext *tclhCtxP,
                               Tclh_PointerTypeTag tag);

/* Function: Tclh_PointerSubtagDefine
 * Registers a tag as a subtype of another tag
 *
 * Parameters:
 * interp - Interpreter. May be NULL.
 * tclhCtxP - Tclh context as returned by <Tclh_LibInit> to use. If NULL,
 *            the Tclh context associated with the interpreter is used.
 * subtagObj - the subtag
 * supertagObj - the super tag
 *
 * The subtag must not have already been registered as a subtag of some
 * other tag.
 *
 * At least one of interp and tclhCtxP must be non-NULL.
 *
 * Returns:
 * A Tcl result code.
 */
TCLH_LOCAL Tclh_ReturnCode Tclh_PointerSubtagDefine(Tcl_Interp *interp,
                                         Tclh_LibContext *tclhCtxP,
                                         Tclh_PointerTypeTag subtagObj,
                                         Tclh_PointerTypeTag supertagObj);

/* Function: Tclh_PointerSubtagRemove
 * Remove a subtag definition
 *
 * Parameters:
 * interp - Interpreter. May be NULL.
 * tclhCtxP - Tclh context as returned by <Tclh_LibInit> to use. If NULL,
 *            the Tclh context associated with the interpreter is used.
 * tagObj - the subtag
 *
 * At least one of interp and tclhCtxP must be non-NULL.
 *
 * Returns:
 * A Tcl result code.
 */
TCLH_LOCAL Tclh_ReturnCode Tclh_PointerSubtagRemove(Tcl_Interp *interp,
                                         Tclh_LibContext *tclhCtxP,
                                         Tclh_PointerTypeTag tagObj);

/* Function: Tclh_PointerSubtags
 * Returns dictionary mapping subtags to their supertags
 *
 * Parameters:
 * interp - Tcl interpreter. May be NULL.
 * tclhCtxP - Tclh context as returned by <Tclh_LibInit> to use. If NULL,
 *            the Tclh context associated with the interpreter is used.
 *
 * At least one of interp and tclhCtxP must be non-NULL.
 *
 * Returns:
 * Dictionary of subtags or NULL on failure.
 */
TCLH_LOCAL Tcl_Obj *Tclh_PointerSubtags(Tcl_Interp *interp,
                                        Tclh_LibContext *tclhCtxP);

/* Function: Tclh_PointerCast
 * Changes the tag associated with a pointer
 *
 * Parameters:
 * interp   - Interpreter in which to store error messages. May be NULL.
 * tclhCtxP - Tclh context as returned by <Tclh_LibInit> to use. If NULL,
 *            the Tclh context associated with the interpreter is used.
 * ptrObjP  - Tcl_Obj holding the wrapped pointer value.
 * newTagObj - new tag to which pointer is to be cast. May be NULL to cast
 *   to void pointer
 * castPtrObj - location to store wrapped pointer after casting
 *
 * At least one of interp and tclhCtxP must be non-NULL.
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
TCLH_LOCAL Tclh_ReturnCode Tclh_PointerCast(Tcl_Interp *interp,
                                 Tclh_LibContext *tclhCtxP,
                                 Tcl_Obj *ptrObj,
                                 Tclh_PointerTypeTag newTagObj,
                                 Tcl_Obj **castPtrObj);

/* Function: Tclh_PointerObjCompare
 * Compares two wrapped pointers for equality
 *
 * Parameters:
 * ip - Interpreter for error messages. May be NULL.
 * ptr1Obj - first wrapped pointer
 * ptr2Obj - second wrapped pointer
 * resultP - where to store result.
 *
 * If both address and tag components are same, stores 1 in resultP. If
 * address is same but not tags, stores -1. Otherwise stores 0.
 *
 * Returns:
 * A Tcl return code.
 */
TCLH_LOCAL Tclh_ReturnCode Tclh_PointerObjCompare(Tcl_Interp *interp,
                                                  Tcl_Obj *ptr1Obj,
                                                  Tcl_Obj *ptr2Obj,
                                                  int *resultP);

/* Function: Tclh_PointerObjDissect
 * Retrieves various characteristics of a wrapped pointer.
 *
 * Parameters:
 * interp   - Tcl interpreter in which the pointer is to be verified.
 *            May be NULL.
 * tclhCtxP - Tclh context as returned by <Tclh_LibInit> to use. If NULL,
 *            the Tclh context associated with the interpreter is used.
 * ptrObj   - Tcl_Obj containing a pointer value to dissect.
 * expectedPtrTag - Type tag for the pointer. Ignored if *tagMatchP* is NULL.
 * pvP      - If not NULL, the pointer value from objP is stored here
 *            on success.
 * ptrTagP  - location to store the pointer tag.
 * tagMatchP - location to store tag matching status. This will be one of
 *            the <Tclh_PointerTagRelation> members. May be NULL.
 * registrationP - location to store registration status. This will be one
 *            of the <Tclh_PointerRegistrationStatus> members. May be NULL.
 *
 * At least one of interp and tclhCtxP must be non-NULL.
 *
 * Returns:
 * TCL_OK    - Success.
 * TCL_ERROR - The passed *ptrObj* was not a wrapped pointer.
 */
TCLH_LOCAL Tclh_ReturnCode
Tclh_PointerObjDissect(Tcl_Interp *interp,
                       Tclh_LibContext *tclhCtxP,
                       Tcl_Obj *ptrObj,
                       Tclh_PointerTypeTag expectedPtrTag,
                       void **pvP,
                       Tclh_PointerTypeTag *ptrTagP,
                       Tclh_PointerTagRelation *tagMatchP,
                       Tclh_PointerRegistrationStatus *registrationP);

/* Function: Tclh_PointerObjInfo
 * Returns a dictionary containing pointer registration information.
 *
 * Parameters:
 * interp - interpreter
 * tclhCtxP - Tclh context as returned by <Tclh_LibInit> to use. If NULL,
 *            the Tclh context associated with the interpreter is used.
 * ptrObj   - Tcl_Obj containing a pointer value to dissect.
 *
 * The returned dictionary always contains the keys `Tag` containing the
 * pointer tag and `Registration` containing one of the values
 * `none`, `safe`, `pinned` or `counted`. If the `Registration` value
 * is not `unregistered`, the dictionary contains the additional keys
 * `RegisteredTag`, holding the tag of the pointer in the registry, and
 * `Match` which may have one of the values `exact`, `derived` or `mismatch`.
 *
 * Returns:
 * A Tcl_Obj dictionary on success, NULL on error.
 */
TCLH_LOCAL Tcl_Obj *Tclh_PointerObjInfo(Tcl_Interp *interp,
                                        Tclh_LibContext *tclhCtxP,
                                        Tcl_Obj *ptrObj);


/* Function: Tclh_ErrorPointerNull
 * Reports an error where a pointer is NULL.
 *
 * Parameters:
 * interp  - Tcl interpreter in which to report the error. May be NULL.
 *
 * The Tcl *errorCode* variable is set to a list of three elements: the
 * *TCLH_EMBEDDER* macro value set by the extension, the literal string
 * *INVALID_VALUE* and the error message.
 *
 * Returns:
 * TCL_ERROR - Always returns this value so caller can just pass on the return
 *             value from this function.
 */
TCLH_LOCAL Tclh_ReturnCode Tclh_ErrorPointerNull(Tcl_Interp *interp);

/* Function: Tclh_ErrorPointerObjType
 * Reports an error where a pointer is the wrong type.
 *
 * Parameters:
 * interp  - Tcl interpreter in which to report the error. May be NULL.
 * ptrObj  - The pointer in question. This is
 *           included in the error message if not *NULL*.
 * tag - Expected tag. This is included in error message if not *NULL*.
 *
 * The Tcl *errorCode* variable is set to a list of three elements: the
 * *TCLH_EMBEDDER* macro value set by the extension, the literal string
 * *WRONG_TYPE* and the error message.
 *
 * Returns:
 * TCL_ERROR - Always returns this value so caller can just pass on the return
 *             value from this function.
 */
TCLH_LOCAL Tclh_ReturnCode Tclh_ErrorPointerObjType(Tcl_Interp *interp,
                                                 Tcl_Obj *ptrObj,
                                                 Tclh_PointerTypeTag tag);

/* Function: Tclh_ErrorPointerObjRegistration
 * Reports an error where a pointer is not registered or registered as the
 * wrong type.
 *
 * Parameters:
 * interp  - Tcl interpreter in which to report the error. May be NULL.
 * ptrObj  - The pointer in question. This is
 *           included in the error message if not *NULL*.
 * registration - registration status code.
 *
 * The Tcl *errorCode* variable is set to a list of three elements: the
 * *TCLH_EMBEDDER* macro value set by the extension, the literal string
 * *WRONG_TYPE* and the error message.
 *
 * Returns:
 * TCL_ERROR - Always returns this value so caller can just pass on the return
 *             value from this function.
 */
TCLH_LOCAL Tclh_ReturnCode
Tclh_ErrorPointerObjRegistration(Tcl_Interp *interp,
                              Tcl_Obj *ptrObj,
                              Tclh_PointerRegistrationStatus regStatus);

#ifdef TCLH_SHORTNAMES
#define PointerLibInit            Tclh_PointerLibInit
#define PointerLibFinit           Tclh_PointerLibFinit
#define PointerRegister           Tclh_PointerRegister
#define PointerUnregister         Tclh_PointerUnregister
#define PointerRegistered         Tclh_PointerRegistered
#define PointerRegistrationAffirm Tclh_PointerRegistrationAffirm
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
#define PointerObjCompare         Tclh_PointerObjCompare
#define PointerPin                Tclh_PointerPin
#define PointerInvalidate         Tclh_PointerInvalidate
#define PointerObjDissect         Tclh_PointerObjDissect
#define PointerObjInfo            Tclh_PointerObjInfo
#define ErrorPointerNull          Tclh_ErrorPointerNull
#define ErrorPointerObjType       Tclh_ErrorPointerObjType
#define ErrorPointerObjRegistration  Tclh_ErrorPointerObjRegistration
#endif

#ifdef TCLH_IMPL
#include "tclhPointerImpl.c"
#endif

#endif /* TCLHPOINTER_H */
