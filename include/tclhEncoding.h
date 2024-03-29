/*
 * Copyright (c) 2021-2023, Ashok P. Nadkarni
 * All rights reserved.
 *
 * See the file LICENSE for license
 */

#ifndef TCLHENCODING_H
#define TCLHENCODING_H

#include "tclhBase.h"

/* Section: Tcl encoding convenience functions
 *
 * Provides some wrappers around Tcl encoding routines.
 */

/* Function: Tclh_EncodingLibInit
 * Must be called to initialize the Encoding module before any of
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
 * Returns:
 * TCL_OK    - Library was successfully initialized.
 * TCL_ERROR - Initialization failed. Library functions must not be called.
 *             An error message is left in the interpreter result.
 */
Tclh_ReturnCode
Tclh_EncodingLibInit(Tcl_Interp *interp, Tclh_LibContext *tclhCtxP);

/* Function: Tclh_ExternalToUtf
 * Wrapper around Tcl_ExternalToUtf to allow lengths > INT_MAX.
 *
 * See documentation of Tcl's Tcl_ExternalToUtf for parameters and
 * return values. The only difference is the types of *srcReadPtr*,
 * *dstWrotePtr* and *dstCharsPtr* which are all of type Tcl_Size*.
 */
#if TCL_MAJOR_VERSION >= 9
int Tclh_ExternalToUtf(Tcl_Interp *interp, Tcl_Encoding encoding,
                       const char *src, Tcl_Size srcLen, int flags,
                       Tcl_EncodingState *statePtr, char *dst,
                       Tcl_Size dstLen, Tcl_Size *srcReadPtr,
                       Tcl_Size *dstWrotePtr, Tcl_Size *dstCharsPtr);
#else
/* Directly call as sizeof(Tcl_Size) == sizeof(int) */
TCLH_INLINE int Tclh_ExternalToUtf(Tcl_Interp *interp, Tcl_Encoding encoding,
                       const char *src, Tcl_Size srcLen, int flags,
                       Tcl_EncodingState *statePtr, char *dst,
                       Tcl_Size dstLen, Tcl_Size *srcReadPtr,
                       Tcl_Size *dstWrotePtr, Tcl_Size *dstCharsPtr) {
    return Tcl_ExternalToUtf(interp, encoding, src, srcLen, flags, statePtr,
                             dst, dstLen, srcReadPtr, dstWrotePtr, dstCharsPtr);
}
#endif

/* Function: Tclh_GetEncodingNulLength
 * Returns the number of bytes used for the terminating nul in an encoding.
 *
 * Parameters:
 * encoding - target encoding
 *
 * Returns:
 * Number of nul bytes used by encoding.
 */
#ifndef TCLH_TCL87API
Tcl_Size Tclh_GetEncodingNulLength (Tcl_Encoding encoding);
#else
TCLH_INLINE Tcl_Size Tclh_GetEncodingNulLength(Tcl_Encoding encoding) {
    return Tcl_GetEncodingNulLength(encoding);
}
#endif

/* Function: Tclh_UtfToExternal
 * Wrapper around Tcl_UtfToExternal to allow lengths > INT_MAX.
 *
 * See documentation of Tcl's Tcl_UtfToExternal for parameters and
 * return values. The only difference is the types of *srcReadPtr*,
 * *dstWrotePtr* and *dstCharsPtr* which are all of type Tcl_Size*.
 */
#if TCL_MAJOR_VERSION >= 9
int Tclh_UtfToExternal(Tcl_Interp *interp, Tcl_Encoding encoding,
                       const char *src, Tcl_Size srcLen, int flags,
                       Tcl_EncodingState *statePtr, char *dst,
                       Tcl_Size dstLen, Tcl_Size *srcReadPtr,
                       Tcl_Size *dstWrotePtr, Tcl_Size *dstCharsPtr);
#else
/* Directly call as sizeof(Tcl_Size) == sizeof(int) */
TCLH_INLINE int Tclh_UtfToExternal(Tcl_Interp *interp, Tcl_Encoding encoding,
                       const char *src, Tcl_Size srcLen, int flags,
                       Tcl_EncodingState *statePtr, char *dst,
                       Tcl_Size dstLen, Tcl_Size *srcReadPtr,
                       Tcl_Size *dstWrotePtr, Tcl_Size *dstCharsPtr) {
    return Tcl_UtfToExternal(interp, encoding, src, srcLen, flags, statePtr,
                              dst, dstLen, srcReadPtr, dstWrotePtr, dstCharsPtr);
}
#endif

/* Function: Tclh_ExternalToUtfAlloc
 * Transforms the input in the given encoding to Tcl's internal UTF-8
 *
 * Parameters:
 * bufPP - location to store pointer to the allocated buffer. This must be
 *    freed by calling Tcl_Free.
 * numBytesOutP - location to store number of bytes copied to the buffer
 *    not counting the terminating nul. May be NULL.
 * 
 * The other parameters are as for Tcl_ExternalToUtfDStringEx. See the Tcl
 * documentation for details. This function differs in that it returns the
 * output in an allocated buffer as opposed to the Tcl_DString structure.
 */
int Tclh_ExternalToUtfAlloc(Tcl_Interp *interp,
                              Tcl_Encoding encoding,
                              const char *src,
                              Tcl_Size srcLen,
                              int flags,
                              char **bufPP,
                              Tcl_Size *numBytesOutP,
                              Tcl_Size *errorLocPtr);

/* Function: Tclh_UtfToExternalAlloc
 * Transforms Tcl's internal UTF-8 encoded data to the given encoding
 *
 * Parameters:
 * bufPP - location to store pointer to the allocated buffer. This must be
 *    freed by calling Tcl_Free.
 * numBytesOutP - location to store number of bytes copied to the buffer
 *    not counting the terminating nul bytes. May be NULL.
 * 
 * The other parameters are as for Tcl_UtfToExternalDStringEx. See the Tcl
 * documentation for details. This function differs in that it returns the
 * output in an allocated buffer as opposed to the Tcl_DString structure.
 */
int Tclh_UtfToExternalAlloc(Tcl_Interp *interp,
                              Tcl_Encoding encoding,
                              const char *src,
                              Tcl_Size srcLen,
                              int flags,
                              char **bufPP,
                              Tcl_Size *numBytesOutP,
                              Tcl_Size *errorLocPtr);

#ifdef TCLH_LIFO_E_SUCCESS /* Only define if Lifo module is available */

/* Function: Tclh_UtfToExternalLifo
 * Transforms Tcl's internal UTF-8 encoded data to the given encoding
 *
 * Parameters:
 * memlifoP - The Tclh_MemLifo from which to allocate memory.
 * numBytesOutP - location to store number of bytes copied to the buffer
 *    not counting the terminating nul bytes. May be NULL.
 *
 * The other parameters are as for Tcl_UtfToExternalDStringEx. See the Tcl
 * documentation for details. This function differs in that it returns the
 * output in memory allocated from a Tclh_Lifo.
 *
 * The *tclhLifo.h* file must be included before *tclhEncoding.h* 
 * for this function to be present.
 */
int Tclh_UtfToExternalLifo(Tcl_Interp *ip,
                           Tcl_Encoding encoding,
                           const char *fromP,
                           Tcl_Size fromLen,
                           int flags,
                           Tclh_Lifo *memlifoP,
                           char **outPP,
                           Tcl_Size *numBytesOutP,
                           Tcl_Size *errorLocPtr);

#endif

/* Function: Tclh_ObjToMultiSzLifo
 * Converts a list of Tcl_Obj to a multi sz string.
 *
 * Parameters:
 * tclhCtxP - Tclh context. May be NULL in which case a temporary Tcl_Encoding
 *    context is used.
 * encoding - Tcl encoding
 * memLifoP - The memlifo from which to allocate memory
 * objP - *Tcl_Obj* containing list of strings
 * flags - TCL_ENCODING_PROFILE_* flags
 * numElemsP - output location to hold the number of strings. May be NULL.
 * numBytesP - output location to hold the number of bytes including
 *             terminating nuls. May be NULL.
 *
 * A multi-sz string is a sequence of nul-terminated strings with an
 * additional nul indicating the end of the string.
 *
 * If the Tcl version supports encoding profiles, the encoding is converted
 * using the replace profile.
 *
 * Returns:
 * Pointer to the MultiSz WCHAR string or NULL on error.
 */
void *Tclh_ObjToMultiSzLifo(Tclh_LibContext *tclhCtxP,
                             Tcl_Encoding encoding,
                             Tclh_Lifo *memLifoP,
                             Tcl_Obj *objP,
                             int flags,
                             Tcl_Size *numElemsP,
                             Tcl_Size *numBytesP
                             );

#ifdef _WIN32
/* Function: Tclh_ObjFromWinChars
 * Returns a Tcl_Obj containing a copy of the passed WCHAR string.
 * 
 * Parameters:
 * tclhCtxP - Tclh context. May be NULL in which case a temporary Tcl_Encoding
 *    context is used.
 * wsP - pointer to a WCHAR string
 * numChars - length of the string. If negative, the string must be nul terminated.
 *
 * If the Tcl version supports encoding profiles, the encoding is converted
 * using the replace profile.
 * 
 * Returns:
 * A non-NULL Tcl_Obj. Panics of failure (memory allocation).
 */
Tcl_Obj *
Tclh_ObjFromWinChars(Tclh_LibContext *tclhCtxP, WCHAR *wsP, Tcl_Size numChars);

/* Function: Tclh_UtfToWinChars
 * Converts a string encoded in Tcl's internal UTF-8 to a WCHAR string
 *
 * Parameters:
 * tclhCtxP - Tclh context. May be NULL in which case a temporary Tcl_Encoding
 *    context is used.
 * srcP - Tcl string to be converted (Tcl internal UTF-8 format)
 * srcLen - length of source string. If negative, must be nul terminated
 * dstP - output buffer
 * dstCapacity - length of output buffer in WCHAR's
 * numCharsP - number of characters stored in output buffer. May be NULL.
 * 
 * Returns:
 * TCL_OK or one of the TCL_ENCODING status codes.
 */
int Tclh_UtfToWinChars(Tclh_LibContext *tclhCtxP,
                       const char *srcP,
                       Tcl_Size srcLen,
                       WCHAR *dstP,
                       Tcl_Size dstCapacity,
                       Tcl_Size *numCharsP
                       );

/* Function: Tclh_ObjToWinCharsAlloc
 * Converts a Tcl_Obj value to a WCHAR string in allocated storage.
 *
 * Parameters:
 * tclhCtxP - Tclh context. May be NULL in which case a temporary Tcl_Encoding
 *    context is used.
 * objP - *Tcl_Obj* to be copied
 * numCharsP - output location to hold the length (in number of characters,
 *     not bytes) of the copied string. May be NULL. The length does not
 *     include the terminating nul WCHAR.
 *
 * If the Tcl version supports encoding profiles, the encoding is converted
 * using the replace profile.
 *
 * Returns:
 * Pointer to the WCHAR string allocated using ckalloc or NULL on error.
 */
WCHAR *Tclh_ObjToWinCharsAlloc(Tclh_LibContext *tclhCtxP,
                               Tcl_Obj *objP,
                               Tcl_Size *numCharsP);

#ifdef TCLH_LIFO_E_SUCCESS /* Only define if Lifo module is available */

/* Function: Tclh_ObjToWinCharsLifo
 * Converts a Tcl_Obj value to a WCHAR string
 *
 * Parameters:
 * tclhCtxP - Tclh context. May be NULL in which case a temporary Tcl_Encoding
 *    context is used.
 * memLifoP - The memlifo from which to allocate memory
 * objP - *Tcl_Obj* to be copied
 * numCharsP - output location to hold the length (in number of characters,
 *     not bytes) of the copied string. May be NULL. The length does not
 *     include the terminating nul WCHAR.
 *
 * If the Tcl version supports encoding profiles, the encoding is converted
 * using the replace profile.
 *
 * Returns:
 * Pointer to the WCHAR string or NULL on error.
 */
WCHAR *Tclh_ObjToWinCharsLifo(Tclh_LibContext *tclhCtxP,
                              Tclh_Lifo *memLifoP,
                              Tcl_Obj *objP,
                              Tcl_Size *numCharsP);

/* Function: Tclh_ObjToWinCharsMultiLifo
 * Converts a list of Tcl_Obj values to a Windows MultiSz WCHAR string.
 *
 * Parameters:
 * tclhCtxP - Tclh context. May be NULL in which case a temporary Tcl_Encoding
 *    context is used.
 * memLifoP - The memlifo from which to allocate memory
 * objP - *Tcl_Obj* to be copied
 * numElemsP - output location to hold the number of strings
 *     May be NULL.
 * numBytesP - Number of bytes including all terminators. May be NULL.
 * If the Tcl version supports encoding profiles, the encoding is converted
 * using the replace profile.
 *
 * Returns:
 * Pointer to the MultiSz WCHAR string or NULL on error.
 */
WCHAR *Tclh_ObjToWinCharsMultiLifo(Tclh_LibContext *tclhCtxP,
                                   Tclh_Lifo *memLifoP,
                                   Tcl_Obj *objP,
                                   Tcl_Size *numElemsP,
                                   Tcl_Size *numBytesP);

/* Function: Tclh_ObjFromWinCharsMulti
 * Converts Windows MultiSz WCHAR string to a Tcl list
 *
 * Parameters:
 * tclhCtxP - Tclh context. May be NULL in which case a temporary Tcl_Encoding
 *    context is used.
 * wsP - pointer to multisz string
 * maxlen - maximum length in bytes.
 *
 * If the Tcl version supports encoding profiles, the encoding is converted
 * using the replace profile.
 *
 * Returns:
 * Pointer to a Tcl_Obj.
 */
Tcl_Obj *Tclh_ObjFromWinCharsMulti(Tclh_LibContext *tclhCtxP,
                                   WCHAR *lpcw,
                                   Tcl_Size maxlen);
#endif /* TCLH_LIFO_E_SUCCESS */

#endif /* _WIN32 */

#ifdef TCLH_SHORTNAMES
# define GetEncodingNulLength Tclh_GetEncodingNulLength
# define ExternalToUtf Tclh_ExternalToUtf
# define UtfToExternal Tclh_UtfToExternal
# define ExternalToUtfAlloc Tclh_ExternalToUtfAlloc
# define UtfToExternalLifo Tclh_UtfToExternalLifo
# define ObjToMultiSzLifo Tclh_ObjToMultiSzLifo
# ifdef _WIN32
#  define ObjFromWinChars Tclh_ObjFromWinChars
#  define ObjToWinCharsLifo Tclh_ObjToWinCharsLifo
#  define ObjToWinCharsMultiLifo Tclh_ObjToWinCharsMultiLifo
# endif
#endif

#ifdef TCLH_IMPL
# include "tclhEncodingImpl.c"
#endif

#endif /* TCLHENCODING_H */
