/* khrplatform.h
 * Minimal KHRONOS platform header to satisfy glad include on systems
 * that don't provide Khronos headers (e.g. some macOS toolchains).
 * This is a small, compatible subset sufficient for glad generated C code.
 */
#ifndef KHR_KHRPLATFORM_H
#define KHR_KHRPLATFORM_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Calling convention macros (empty on most platforms) */
#ifndef KHRONOS_APICALL
#define KHRONOS_APICALL
#endif
#ifndef KHRONOS_APIENTRY
#define KHRONOS_APIENTRY
#endif
#ifndef KHRONOS_APIATTRIBUTES
#define KHRONOS_APIATTRIBUTES
#endif


/* Fixed-width integer types used by Khronos headers. Prefer <stdint.h>.
 * Provide fallbacks for older MSVC compilers.
 */
#if defined(_MSC_VER) && _MSC_VER < 1600
typedef unsigned __int8  khronos_uint8_t;
typedef signed   __int8  khronos_int8_t;
typedef unsigned __int16 khronos_uint16_t;
typedef signed   __int16 khronos_int16_t;
typedef unsigned __int32 khronos_uint32_t;
typedef signed   __int32 khronos_int32_t;
typedef signed   __int64 khronos_int64_t;
typedef unsigned __int64 khronos_uint64_t;
#else
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
typedef uint8_t  khronos_uint8_t;
typedef int8_t   khronos_int8_t;
typedef uint16_t khronos_uint16_t;
typedef int16_t  khronos_int16_t;
typedef uint32_t khronos_uint32_t;
typedef int32_t  khronos_int32_t;
typedef int64_t  khronos_int64_t;
typedef uint64_t khronos_uint64_t;
#endif

/* Floating point type used by Khronos headers */
typedef float khronos_float_t;

/* Pointer-sized and ssize types */
#include <stddef.h>
#if defined(_WIN32) && !defined(__MINGW32__)
typedef intptr_t khronos_intptr_t;
typedef ptrdiff_t khronos_ssize_t;
#else
/* ssize_t and intptr_t available on POSIX systems */
typedef intptr_t khronos_intptr_t;
typedef ssize_t  khronos_ssize_t;
#endif

typedef khronos_uint8_t khronos_boolean_enum_t;

#ifdef __cplusplus
}
#endif

#endif /* KHR_KHRPLATFORM_H */
