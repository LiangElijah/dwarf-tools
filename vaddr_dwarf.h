#ifndef __VADDR_DWARF_H__
#define __VADDR_DWARF_H__

#include <cstdio>  // printf ...
#include <cstdint> // uint32_t ...
#include <cstdlib> // malloc
#include <cerrno>  // errno
#include <cstring> // strerror

extern "C" {
    #include <unistd.h> // getopt、access

    #include "list.h"
    #include "dwarf_api.h"
    #include "vaddr_file.h"
    #include "vaddr_string.h"
}

enum RowType
{
    RowType_HEAD,
    RowType_UNIT,
    RowType_VAR,
    RowType_TYPE,
    RowType_MEM,
};

struct Vaddr_list
{
    struct list_head row;
    struct list_head column;
    int8_t name[256];
    enum RowType row_type;
    
    union
    {
        struct Unit
        {
            // ...
        } u;
        struct Var
        {
            uint32_t num;
            uint64_t size;
            uint32_t deep[4];

            int32_t operation[4];
            
            uint8_t declaration;
        } v;
        struct Type
        {
            // ...
        } t;
        struct Mem
        {
            uint32_t num;
            uint64_t size;
            uint32_t deep[4];

            int32_t operation[4];
            
            uint64_t bit_offset;
            uint64_t bit_size;
        } m;
    } u;
};

class Vaddr_Dwarf
{
private:
    char is_ready = 0;
    Vaddr_File *file;
    Vaddr_list head;

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
    int get_dwarf_flag(Dwarf_Debug dbg, Dwarf_Die die, struct Vaddr_list *node, Dwarf_Error *error);
    int get_dwarf_bit(Dwarf_Debug dbg, Dwarf_Die die, struct Vaddr_list *node, Dwarf_Error *error);
    int get_dwarf_loc(Dwarf_Debug dbg, Dwarf_Die die, struct Vaddr_list *node, Dwarf_Error *error);
    int get_dwarf_name(Dwarf_Debug dbg, Dwarf_Die die, struct Vaddr_list *node, Dwarf_Error *error);
    int scan_type_dwarf(Dwarf_Debug dbg, Dwarf_Die parent_die, struct Vaddr_list *head, Dwarf_Error *error);
    int scan_dwarf(Dwarf_Debug dbg, struct Vaddr_list *head, Dwarf_Error *error);
    int print_mem_dwarf(Dwarf_Debug dbg, struct Vaddr_list *head, Dwarf_Error *error, uint8_t *tab);
    int print_type_dwarf(Dwarf_Debug dbg, struct Vaddr_list *head, Dwarf_Error *error, uint8_t *tab);
    int print_var_dwarf(Dwarf_Debug dbg, struct Vaddr_list *head, Dwarf_Error *error);
    int print_dwarf(Dwarf_Debug dbg, struct Vaddr_list *head, Dwarf_Error *error);
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
