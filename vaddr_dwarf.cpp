#include "vaddr_dwarf.h"

/* METHOD */
const Dwarf_Obj_Access_Methods_a Vaddr_Dwarf::methods = {
    Vaddr_Dwarf::om_get_section_info,
    Vaddr_Dwarf::om_get_byte_order,
    Vaddr_Dwarf::om_get_length_size,
    Vaddr_Dwarf::om_get_pointer_size,
    Vaddr_Dwarf::om_get_filesize,
    Vaddr_Dwarf::om_get_section_count,
    Vaddr_Dwarf::om_load_section,
    Vaddr_Dwarf::om_relocate_a_section
};

int Vaddr_Dwarf::om_get_section_info(void *obj, Dwarf_Unsigned section_index, Dwarf_Obj_Access_Section_a *return_section, int *error)
{
    Vaddr_Dwarf *dwarf_p = (Vaddr_Dwarf *)(obj);

    *error = 0; /* No error. Avoids unused arg */
    if (section_index >= dwarf_p->file->semnum) {
        return DW_DLV_NO_ENTRY;
    }

    return_section->as_name   = dwarf_p->file->sems[section_index].name;
    return_section->as_type   = 0;
    return_section->as_flags  = 0;
    return_section->as_addr   = 0;
    return_section->as_offset = 0;
    return_section->as_size   = dwarf_p->file->sems[section_index].size;
    return_section->as_link   = 0;
    return_section->as_info   = 0;
    return_section->as_addralign = 0;
    return_section->as_entrysize = 1;

    return DW_DLV_OK;
}

Dwarf_Small Vaddr_Dwarf::om_get_byte_order(void *obj)
{
    Vaddr_Dwarf *dwarf_p = (Vaddr_Dwarf *)(obj);

    return dwarf_p->file->byte_order;
}

Dwarf_Small Vaddr_Dwarf::om_get_length_size(void *obj)
{
    Vaddr_Dwarf *dwarf_p = (Vaddr_Dwarf *)(obj);

    return dwarf_p->file->length_size;
}

Dwarf_Small Vaddr_Dwarf::om_get_pointer_size(void *obj)
{
    Vaddr_Dwarf *dwarf_p = (Vaddr_Dwarf *)(obj);

    return dwarf_p->file->pointer_size;
}

Dwarf_Unsigned Vaddr_Dwarf::om_get_filesize(void *obj)
{
    Vaddr_Dwarf *dwarf_p = (Vaddr_Dwarf *)(obj);

    return dwarf_p->file->file_size;
}

Dwarf_Unsigned Vaddr_Dwarf::om_get_section_count(void *obj)
{
    Vaddr_Dwarf *dwarf_p = (Vaddr_Dwarf *)(obj);

    return dwarf_p->file->semnum;
}

int Vaddr_Dwarf::om_load_section(void *obj, Dwarf_Unsigned section_index, Dwarf_Small **return_data, int *error)
{
    Vaddr_Dwarf *dwarf_p = (Vaddr_Dwarf *)(obj);

    *error = 0; /* No error. Avoids unused arg */
    if (section_index >= dwarf_p->file->semnum) {
        return DW_DLV_NO_ENTRY;
    }

    *return_data = (Dwarf_Small *)(dwarf_p->file->sems[section_index].data);

    return DW_DLV_OK;
}

int Vaddr_Dwarf::om_relocate_a_section(void* obj, Dwarf_Unsigned section_index, Dwarf_Debug dbg, int *error)
{
    Vaddr_Dwarf *dwarf_p = (Vaddr_Dwarf *)(obj);

    *error = 0; /* No error. Avoids unused arg */
    if (section_index >= dwarf_p->file->semnum) {
        return DW_DLV_NO_ENTRY;
    }

    /* Do Something here */

    return DW_DLV_OK;
}

/* CLASS */
Vaddr_Dwarf::Vaddr_Dwarf(Vaddr_File *file)
{
    connect_file(file);
}

Vaddr_Dwarf::Vaddr_Dwarf()
{
    is_ready = 0;
}

Vaddr_Dwarf::~Vaddr_Dwarf()
{

}

int Vaddr_Dwarf::connect_file(Vaddr_File *file)
{
    is_ready = 0;

    if((file != NULL) && (file->ready()))
    {
        this->file = file;
        is_ready = 1;
    }

    return is_ready;
}

int Vaddr_Dwarf::ready()
{
    return is_ready;
}

void Vaddr_Dwarf::print()
{

}

int Vaddr_Dwarf::analyze(Vaddr_String *str)
{
    if((str == NULL) || (str->ready() != 1) || (this->ready() != 1)) {
        printf("[%s-%s:%d] Something not ready.\n", __FILE__, __func__, __LINE__);
        return DW_DLV_ERROR;
    }

    Dwarf_Obj_Access_Interface_a dw_interface = {this, &methods};

    Dwarf_Debug dbg = NULL;
    Dwarf_Error error = NULL;

    Dwarf_Die var_die  = NULL;
    Dwarf_Die type_die = NULL;

    Dwarf_Unsigned ret_size = 0;
    Dwarf_Unsigned ret_offset = 0;
    Dwarf_Half attribute = 0;

    char *diename = NULL;
    int res = 0;
    
    // 1、创建 dwarf 对象
    if(file->file_type == FILE_COFF) {
        res = ASSERT(dwarf_object_init_b, &dw_interface, NULL, NULL, DW_GROUPNUMBER_ANY, &dbg, &error);
    } else if(file->file_type == FILE_ELF) {
        res = ASSERT(dwarf_init_path, file->file_path, NULL, 0, DW_GROUPNUMBER_ANY, NULL, NULL, &dbg, &error);
    } else {
        printf("[%s-%s:%d] Unsupported File Type (%d).\n", __FILE__, __func__, __LINE__, file->file_type);
    }
    if(res != DW_DLV_OK) goto RET;

    // 2、查找 variable 变量
    res = ASSERT(find_variable, dbg, str->vstr_part[0], &var_die, &error);
    if(res != DW_DLV_OK) goto FIN;

    // 3、查找 variable 基本类型
    res = ASSERT(dwarf_die_basic_type, dbg, var_die, &type_die, &error);
    if(res != DW_DLV_OK) goto VAR;

    // 4、判断 variable 是否数组
    res = ASSERT(dwarf_die_is_array, dbg, var_die, &error);
    if(res == DW_DLV_OK) {
        // 4.1、获取 variable 数组维度
        res = ASSERT(dwarf_array_info, dbg, var_die, str->dms_file[0].deep, &str->dms_file[0].num, &error);
        if(res != DW_DLV_OK) goto TYPE;

        // 4.2、获取 variable 类型大小
        res = ASSERT(dwarf_bytesize, type_die, &ret_size, &error);
        if(res != DW_DLV_OK) goto TYPE;
        str->dms_file[0].size = ret_size;
    } else if(res != DW_DLV_NOT_CMP) {
        goto TYPE;
    }

    // 5、获取 variable 地址信息
    res = ASSERT(dwarf_loc_info, dbg, var_die, str->operation[0], &error);
    if(res != DW_DLV_OK) goto TYPE;

    // 6、member 变量
    for(int next = 1; next < str->vstr_part_num; next++) {
        dwarf_dealloc_die(var_die);
        var_die = NULL;

        // 6.1、查找 member 变量
        res = ASSERT(find_member, dbg, type_die, str->vstr_part[next], &var_die, &error);
        if(res != DW_DLV_OK) goto TYPE;

        dwarf_dealloc_die(type_die);
        type_die = NULL;

        // 6.2、查找 member 基本类型
        res = ASSERT(dwarf_die_basic_type, dbg, var_die, &type_die, &error);
        if(res != DW_DLV_OK) goto TYPE;

        // 6.3、判断 member 是否数组
        res = ASSERT(dwarf_die_is_array, dbg, var_die, &error);
        if(res == DW_DLV_OK) {
            // 6.3.1、获取 member 数组维度
            res = ASSERT(dwarf_array_info, dbg, var_die, str->dms_file[next].deep, &str->dms_file[next].num, &error);
            if(res != DW_DLV_OK) goto TYPE;

            // 6.3.2、获取 member 类型大小
            res = ASSERT(dwarf_bytesize, type_die, &ret_size, &error);
            if(res != DW_DLV_OK) goto TYPE;
            str->dms_file[next].size = ret_size;
        } else if(res != DW_DLV_NOT_CMP) {
            goto TYPE;
        }

        // 6.4、获取 member 地址信息
        res = ASSERT(dwarf_loc_info, dbg, var_die, str->operation[next], &error);
        if(res != DW_DLV_OK) goto TYPE;
    }

    // 7、获取 variable 地址信息
    res = dwarf_bitsize(var_die, &ret_size, &error);
    if(res == DW_DLV_ERROR) {
        printf("[%s-%s:%d] dwarf_bitsize() %s.\n", __FILE__, __func__, __LINE__, dwarf_errmsg(error));
        goto TYPE;
    } else if (res == DW_DLV_NO_ENTRY) {
        str->bitsize = 0;
    }
    else {
        str->bitsize = ret_size;
    }

    // 8、获取 variable 地址信息
    res = dwarf_bitoffset(var_die, &attribute, &ret_offset, &error);
    if(res == DW_DLV_ERROR) {
        printf("[%s-%s:%d] dwarf_bitoffset() %s.\n", __FILE__, __func__, __LINE__, dwarf_errmsg(error));
        goto TYPE;
    } else if (res == DW_DLV_NO_ENTRY) {
        str->bitoffset = 0;
    } else {
        str->bitoffset = ret_offset;
    }

    // 8、获取 variable 类型字符串
    res = ASSERT(dwarf_diename, type_die, &diename, &error);
    if(res == DW_DLV_OK) {
        memcpy(str->vtype, diename, strlen(diename) + 1);
        str->is_analyzed = 1;
    }

TYPE:
    if(type_die != NULL) dwarf_dealloc_die(type_die);
VAR:
    if(var_die != NULL) dwarf_dealloc_die(var_die);
FIN:
    if(file->file_type == FILE_COFF) {
        dwarf_object_finish(dbg);
    } else if(file->file_type == FILE_ELF) {
        dwarf_finish(dbg);
    }
RET:
    return res;
}

int Vaddr_Dwarf::search_tag(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Half tag, const char *name, Dwarf_Error *error)
{
    Dwarf_Half ret_tag = 0;
    char *diename = NULL;
    
    int res = 0;

    res = ASSERT2(dwarf_tag, die, &ret_tag, error);
    if(res != DW_DLV_OK) return res;

    if(tag != ret_tag) return DW_DLV_NOT_CMP;

    res = ASSERT2(dwarf_diename, die, &diename, error);
    if(res != DW_DLV_OK) return res;

    if(strcmp(name, diename) != 0) return DW_DLV_NOT_CMP;

    return DW_DLV_OK;
}

int Vaddr_Dwarf::find_variable(Dwarf_Debug dbg, const char *name, Dwarf_Die *die, Dwarf_Error *error)
{
    Dwarf_Die cu_die = NULL;
    Dwarf_Die var_die = NULL;
    Dwarf_Die next_die = NULL;
    Dwarf_Bool flag = 0;

    int res = 0;

    while(1) {
        res = ASSERT2(dwarf_next_cu_die, dbg, &cu_die, error);
        if(res != DW_DLV_OK) goto RET;

        res = dwarf_child(cu_die, &var_die, error);
        if(res == DW_DLV_ERROR) {
            printf("[%s-%s:%d] dwarf_child() %s.\n", __FILE__, __func__, __LINE__, dwarf_errmsg(*error));
            goto CU;
        } else if (res == DW_DLV_NO_ENTRY){
            continue;
        }

        res = ASSERT2(search_tag, dbg, var_die, DW_TAG_variable, name, error);
        if(res == DW_DLV_OK) {
            res = dwarf_die_flag(dbg, var_die, DW_AT_declaration, &flag, error);
            if (res == DW_DLV_ERROR) {
                printf("[%s-%s:%d] dwarf_die_flag() %s.\n", __FILE__, __func__, __LINE__, dwarf_errmsg(*error));
                goto VAR;
            } else if ((res == DW_DLV_NO_ENTRY) || (flag == 0)) {
                dwarf_dealloc_die(cu_die);
                goto OK;
            }
        }
        else if(res != DW_DLV_NOT_CMP) 
        {
            goto VAR;
        }

        while(1)
        {
            res = dwarf_siblingof_b(dbg, var_die, TRUE, &next_die, error);
            if(res == DW_DLV_ERROR) {
                printf("[%s-%s:%d] dwarf_siblingof_b() %s.\n", __FILE__, __func__, __LINE__, dwarf_errmsg(*error));
                goto VAR;
            } else if (res == DW_DLV_NO_ENTRY){
                break;
            }

            dwarf_dealloc_die(var_die);
            var_die = next_die;

            res = ASSERT2(search_tag, dbg, var_die, DW_TAG_variable, name, error);
            if(res == DW_DLV_OK)
            {
                res = dwarf_die_flag(dbg, var_die, DW_AT_declaration, &flag, error);
                if (res == DW_DLV_ERROR){
                    printf("[%s-%s:%d] dwarf_die_flag() %s.\n", __FILE__, __func__, __LINE__, dwarf_errmsg(*error));
                    goto VAR;
                } else if ((res == DW_DLV_NO_ENTRY) || (flag == 0)) {
                    dwarf_dealloc_die(cu_die);
                    goto OK;
                }
            }
            else if(res != DW_DLV_NOT_CMP) 
            {
                goto VAR;
            }
        }

        dwarf_dealloc_die(cu_die);
        dwarf_dealloc_die(var_die);
    }

OK:
    *die = var_die;
    return DW_DLV_OK;

VAR:
    dwarf_dealloc_die(var_die);
CU:
    dwarf_dealloc_die(cu_die);
RET:
    return res;
}

int Vaddr_Dwarf::find_member(Dwarf_Debug dbg, Dwarf_Die parent_die, const char *name, Dwarf_Die *die, Dwarf_Error *error)
{
    Dwarf_Die mem_die = 0;
    Dwarf_Die next_die = 0;

    int res = 0;

    res = ASSERT2(dwarf_child, parent_die, &mem_die, error);
    if(res != DW_DLV_OK) goto RET;

    res = ASSERT2(search_tag, dbg, mem_die, DW_TAG_member, name, error);
    if(res == DW_DLV_OK) {
        *die = mem_die;
        return DW_DLV_OK;
    } else if(res != DW_DLV_NOT_CMP) {
        goto MEM;
    }

    while(1) {
        res = ASSERT2(dwarf_siblingof_b, dbg, mem_die, TRUE, &next_die, error);
        if(res != DW_DLV_OK) goto MEM;

        dwarf_dealloc_die(mem_die);
        mem_die = next_die;
        next_die = NULL;

        res = ASSERT2(search_tag, dbg, mem_die, DW_TAG_member, name, error);
        if(res == DW_DLV_OK) {
            *die = mem_die;
            return DW_DLV_OK;
        } else if(res != DW_DLV_NOT_CMP) {
            break;
        }
    }

MEM:
    dwarf_dealloc_die(mem_die);
RET:
    return res;
}
