#ifndef _ZENO_SPEC_SRC_SUPPORT_DEFS_H
#define _ZENO_SPEC_SRC_SUPPORT_DEFS_H

/* Check Standard C version. */
#ifdef __STDC_VERSION__
    #define HAVE_C99 (__STDC_VERSION__ >= 199901L)
#endif

/* Check POSIX version. */
#ifdef __unix__
    #include <unistd.h>
    #ifdef _POSIX_VERSION
        #define HAVE_POSIX_2001 (_POSIX_VERSION >= 200112L)
    #endif
#endif

/* Boolean constants */
#define true 1
#define false 0

/* C99 inline.*/
#if !HAVE_C99
    #if defined(__GNUC__)
        #define inline __inline__
    #else
        #define inline
    #endif
#endif

/* unused attribute */
#if defined(__has_attribute)
    #if __has_attribute(unused)
        #define ATTR_UNUSED __attribute__((unused))
    #endif
#endif
#ifndef ATTR_UNUSED
    #define ATTR_UNUSED
#endif

#endif
