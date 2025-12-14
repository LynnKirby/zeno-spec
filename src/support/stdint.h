#ifndef _ZENOC_SRC_SUPPORT_STDINT_H
#define _ZENOC_SRC_SUPPORT_STDINT_H

#include "src/support/base.h"

#if HAVE_C99 || HAVE_POSIX_2001
    #include <stdint.h>
#else
    #error "implement me"
#endif

#endif
