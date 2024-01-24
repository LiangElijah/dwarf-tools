#ifndef __DWARF_API_H__
#define __DWARF_API_H__

#include <stdio.h>
#include <stdint.h>

#include "dwarf.h"
#include "libdwarf.h"
#include "libdwarf_private.h"
#include "config.h"

#define DW_DLV_NOT_CMP -2

#define ASSERT(func, ...) \
({ \
    int res = func(__VA_ARGS__); \
    if (res == DW_DLV_ERROR) { \
        printf("[%s-%s:%d] %s() %s.\n", __FILE__, __func__, __LINE__, #func, dwarf_errmsg(error)); \
    } else if (res == DW_DLV_NO_ENTRY) { \
        printf("[%s-%s:%d] %s() No Entry.\n", __FILE__, __func__, __LINE__, #func); \
    } \
    res; \
})

#define ASSERT2(func, ...) \
({ \
    int res = func(__VA_ARGS__); \
    if (res == DW_DLV_ERROR) { \
        printf("[%s-%s:%d] %s() %s.\n", __FILE__, __func__, __LINE__, #func, dwarf_errmsg(*error)); \
    } else if (res == DW_DLV_NO_ENTRY) { \
        printf("[%s-%s:%d] %s() No Entry.\n", __FILE__, __func__, __LINE__, #func); \
    } \
    res; \
})

/* DIE API */
int dwarf_die_udata(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Half attrnum, Dwarf_Unsigned *ret_udata, Dwarf_Error *error);
int dwarf_die_sdata(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Half attrnum, Dwarf_Signed *ret_sdata, Dwarf_Error *error);
int dwarf_die_string(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Half attrnum, char **string, Dwarf_Error *error);
int dwarf_die_flag(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Half attrnum, Dwarf_Bool *ret_flag, Dwarf_Error *error);
int dwarf_die_type(Dwarf_Debug dbg, Dwarf_Die main_die, Dwarf_Die *type_die, Dwarf_Error *error);
int dwarf_die_basic_type(Dwarf_Debug dbg, Dwarf_Die main_die, Dwarf_Die *type_die, Dwarf_Error *error);
/* DIE API 2 */
int dwarf_die_is_tag(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Half tag, Dwarf_Error *error);
int dwarf_die_is_array(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Error *error);
/* ATTR API */
int dwarf_attr_loc(Dwarf_Debug dbg, Dwarf_Attribute attr, int32_t *operation, Dwarf_Error *error);
/* ATTR API 2 */
int dwarf_attr_is_locexpr(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Attribute attr, Dwarf_Error *error);
/* COMMON API */
void dwarf_dealloc_list(Dwarf_Debug dbg, Dwarf_Attribute *attrbuf, Dwarf_Signed attrcount);
int dwarf_next_cu_die(Dwarf_Debug dbg, Dwarf_Die *cu_die, Dwarf_Error *error);
int dwarf_array_info(Dwarf_Debug dbg, Dwarf_Die die, uint32_t *array, uint32_t *num, Dwarf_Error *error);
int dwarf_loc_info(Dwarf_Debug dbg, Dwarf_Die die, int32_t *operation, Dwarf_Error *error);

#endif /* __DWARF_API_H__ */