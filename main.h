#ifndef __MAIN_H__
#define __MAIN_H__

#include <cstdio>  // printf ...
#include <cstdint> // uint32_t ...
#include <cstdlib> // malloc
#include <cerrno>  // errno
#include <cstring> // strerror

extern "C" {
   #include <unistd.h> // getopt、access

   #include "dwarf_die.h"
   #include "dwarf_method.h"
   #include "dwarf_elf.h"
   #include "dwarf_coff.h"
   #include "dwarf_str.h"
   #include "dwarf_addr.h"
}

#endif /* __MAIN_H__ */
