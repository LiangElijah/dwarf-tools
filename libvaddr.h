#ifndef _LIBVADDR_H
#define _LIBVADDR_H

#ifdef VADDR_API
#   undef VADDR_API
#endif /* VADDR_API */

#ifndef LIBVADDR_STATIC
#   if defined(_WIN32) || defined(__CYGWIN__)
#       ifdef LIBDWARF_BUILD
#           define VADDR_API __declspec(dllexport)
#       else /* !LIBDWARF_BUILD */
#           define VADDR_API __declspec(dllimport)
#       endif /* LIBDWARF_BUILD */
#   elif (defined(__SUNPRO_C)  || defined(__SUNPRO_CC))
#       if defined(PIC) || defined(__PIC__)
#           define VADDR_API __global
#       endif /* __PIC__ */
#   elif (defined(__GNUC__) && __GNUC__ >= 4) || \
        defined(__INTEL_COMPILER)
#       if defined(PIC) || defined(__PIC__)
#           define VADDR_API __attribute__ ((visibility("default")))
#       endif /* PIC */
#   endif /* WIN32 SUNPRO GNUC  */
#endif /* !LIBVADDR_STATIC */

#ifndef VADDR_API
#   define VADDR_API
#endif /* VADDR_API */

#endif /* _LIBVADDR_H */