#ifndef __DWARF_ELF_H__
#define __DWARF_ELF_H__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "dwarf.h"
#include "libdwarf.h"

#include "dwarf_method.h"
#include "elf.h"

#define LZSS_EOD 0x0FFF // 假设 12 位高位全 1 为结束符

int dwarf_elf_init(const char *path, 
    Dwarf_Obj *initSec_p,
    Dwarf_Debug *ret_dbg, 
    Dwarf_Error *error);
void dwarf_elf_deinit(Dwarf_Debug dw_dbg);

#endif /* __DWARF_ELF_H__ */