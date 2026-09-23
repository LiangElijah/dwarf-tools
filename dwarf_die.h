#ifndef __DWARF_DIE_H__
#define __DWARF_DIE_H__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "dwarf.h"
#include "dwarf_elf_defines.h"
#include "libdwarf.h"
#include "libdwarf_private.h"
#include "config.h"
#include "list.h"

typedef enum {
    NodeTyp_CU,
    NodeTyp_VAR,
    NodeTyp_TYPE,
    NodeTyp_MEM,
    NodeTyp_SRT,
} en_nodeType_t;

typedef struct st_dieNode {
    list_head_t row; // 行
    list_head_t column; // 列

    en_nodeType_t nodeType;
    union {
        struct {
            char *name;
        } cu;
        struct {
            char *name;
            int32_t operation[4];
        } var;
        struct {
            char *name;
            char *name_typedef;
            uint64_t byte_size;
            uint32_t dimension[4];
            uint32_t dimensionNum;
            bool pointer_type;
            bool reference_type;
        } type;
        struct {
            char *name;
            int32_t operation[4];
            uint64_t bit_offset;
            uint64_t bit_size;
        } mem;
        struct {
            uint16_t index;
        } srt;
    } un;
} st_dieNode_t;

int dwarf_die_init(Dwarf_Debug dw_dbg, 
    st_dieNode_t **entry, 
    Dwarf_Error *error);
int dwarf_die_deinit(Dwarf_Debug dw_dbg,
    st_dieNode_t *entry,
    Dwarf_Error *error);
int dwarf_print_die(Dwarf_Debug dw_dbg, 
    st_dieNode_t *entry, 
    Dwarf_Error *error);

#endif /* __DWARF_DIE_H__ */