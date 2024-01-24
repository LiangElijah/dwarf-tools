#ifndef __VADDR_DWARF_H__
#define __VADDR_DWARF_H__

#include <cstdio>  // printf ...
#include <cstdint> // uint32_t ...
#include <cstdlib> // malloc
#include <cerrno>  // errno
#include <cstring> // strerror

extern "C" {
    #include <unistd.h> // getopt、access

    #include "dwarf_api.h"
    #include "vaddr_file.h"
    #include "vaddr_string.h"
}

class Vaddr_Dwarf
{
private:
    char is_ready = 0;
    Vaddr_File *file;

    static const Dwarf_Obj_Access_Methods_a methods;
    static int om_get_section_info(void *obj, Dwarf_Unsigned section_index, Dwarf_Obj_Access_Section_a *return_section, int *error);
    static Dwarf_Small om_get_byte_order(void *obj);
    static Dwarf_Small om_get_length_size(void *obj);
    static Dwarf_Small om_get_pointer_size(void *obj);
    static Dwarf_Unsigned om_get_filesize(void *obj);
    static Dwarf_Unsigned om_get_section_count(void *obj);
    static int om_load_section(void *obj, Dwarf_Unsigned section_index, Dwarf_Small **return_data, int *error);
    static int om_relocate_a_section(void* obj, Dwarf_Unsigned section_index, Dwarf_Debug dbg, int *error);

    int search_tag(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Half tag, const char *name, Dwarf_Error *error);
    int find_variable(Dwarf_Debug dbg, const char *name, Dwarf_Die *die, Dwarf_Error *error);
    int find_member(Dwarf_Debug dbg, Dwarf_Die parent_die, const char *name, Dwarf_Die *die, Dwarf_Error *error);

public:
    Vaddr_Dwarf(Vaddr_File *file);
    Vaddr_Dwarf();
    ~Vaddr_Dwarf();

    int connect_file(Vaddr_File *file);
    int ready();
    void print();
    int analyze(Vaddr_String *str);
};

#endif /* __VADDR_DWARF_H__ */
