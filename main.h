#ifndef __MAIN_H__
#define __MAIN_H__

#include <cstdio>  // printf ...
#include <cstdint> // uint32_t ...
#include <cstdlib> // malloc
#include <cerrno>  // errno
#include <cstring> // strerror

extern "C" {
   #include <unistd.h> // getopt、access

   #include "vaddr_string.h"
   #include "vaddr_file.h"
   #include "vaddr_dwarf.h"
}

#endif /* __MAIN_H__ */
