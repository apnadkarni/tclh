#ifndef TCLHNAMESPACE_H
#define TCLHNAMESPACE_H

/*
 * Copyright (c) 2022, Ashok P. Nadkarni
 * All rights reserved.
 *
 * See the file LICENSE for license
 */

#include "tclhBase.h"

/* Section: Namespace utilities
 *
 * Provides utility routines related to Tcl namespaces.
 */

/* Function: Tclh_NsLibInit
 * Must be called to initialize the Namespace module before any of
 * the other functions in the module.
 *
 * Parameters:
 * interp - Tcl interpreter for error messages. May be NULL.
 * tclhCtxP - Tclh context as returned by <Tclh_LibInit> to use. If NULL,
 *    the Tclh context associated with the interpreter is used after
 *    initialization if necessary.
 *
 * Returns:
 * TCL_OK    - Library was successfully initialized.
 * TCL_ERROR - Initialization failed. Library functions must not be called.
 *             An error message is left in the interpreter result.
 */
TCLH_LOCAL Tclh_ReturnCode Tclh_NsLibInit(Tcl_Interp *interp,
                                          Tclh_LibContext *tclhCtxP);

/* Function: Tclh_NsIsGlobalNs
 * Returns true if passed namespace is the global namespace.
 *
 * Parameters:
 * nsP - namespace name
 *
 * Any name consisting only of two or more ':' characters is considered
 * the global namespace.
 *
 * Returns:
 * 1 if the passed name is the name of the global namespace
 * and 0 otherwise.
 */
TCLH_INLINE int Tclh_NsIsGlobalNs(const char *nsP)
{
    const char *p = nsP;
    while (*p == ':')
        ++p;
    return (*p == '\0') && ((p - nsP) >= 2);
}

/* Function: Tclh_NsIsFQN
 * Returns true if the given name is fully qualified.
 *
 * Parameters:
 * nameP - name to check
 *
 * Returns:
 * 1 if the passed name is fully qualified and 0 otherwise.
 */
TCLH_INLINE int Tclh_NsIsFQN(const char *nsP)
{
    return nsP[0] == ':' && nsP[1] == ':';
}

/* Function: Tclh_NsQualifyNameObj
 * Returns a fully qualified name
 *
 * Parameters:
 * ip - interpreter. This may be NULL iff defaultNsP is not NULL.
 * nameObj - Name to be qualified
 * defaultNsP - namespace to use to qualify if necessary.
 *   This must be fully qualified. If NULL, the current namespace will be used.
 *
 * The passed *Tcl_Obj* is fully qualified with the current namespace if
 * not already a FQN.
 *
 * The returned *Tcl_Obj* may be the same as *nameObj* if it is already
 * fully qualified or a newly allocated one. In either case, no changes in
 * reference counts is done and left to the caller. In particular, when
 * releasing the Tcl_Obj, caller must increment the reference count before
 * decrementing it to correctly handle the case where the existing object
 * is returned.
 *
 * Returns:
 * A *Tcl_Obj* containing the fully qualified name.
 */
TCLH_LOCAL Tcl_Obj *
Tclh_NsQualifyNameObj(Tcl_Interp *ip, Tcl_Obj *nameObj, const char *defaultNsP);

/* Function: Tclh_NsQualifyName
 * Fully qualifies a name.
 *
 * Parameters:
 * ip - interpreter. This may be NULL iff defaultNsP is not NULL.
 * nameP - name to be qualified
 * nameLen - length of the name excluding nul terminator. If negative,
 *    nameP must be nul terminated.
 * dsP - storage to use if necessary. This is initialized by the function
 *   and must be freed with Tcl_DStringFree on return in all cases.
 * defaultNsP - namespace to use to qualify if necessary.
 *   This must be fully qualified. If NULL, the current namespace will be used.
 *
 * The passed name is fully qualified with the current namespace if
 * not already a FQN.
 *
 * Returns:
 * A pointer to a fully qualified name. This may be either nameP or a
 * pointer into dsP. Caller should not assume eiter.
 */
TCLH_LOCAL const char *Tclh_NsQualifyName(Tcl_Interp *ip,
                                          const char *nameP,
                                          Tcl_Size nameLen,
                                          Tcl_DString *dsP,
                                          const char *defaultNsP);

/* Function: Tclh_NsTailPos
 * Returns the index of the tail component in a name.
 *
 * Parameters:
 * nameP - the name to parse
 *
 * If there are no qualifiers, the returned index will be 0 corresponding to
 * the start of the name. If the name ends in a namespace separator, the
 * index will be that of the terminating nul.
 *
 * Returns:
 * Index of the tail component.
 */
TCLH_LOCAL int Tclh_NsTailPos(const char *nameP);

#ifdef TCLH_SHORTNAMES

#define NsIsGlobalNs  Tclh_NsIsGlobalNs
#define NsIsFQN       Tclh_NsIsFQN
#define NsQualifyName Tclh_NsQualifyName
#endif

#ifdef TCLH_IMPL
#include "tclhNamespaceImpl.c"
#endif

#endif /* TCLHNAMESPACE_H */