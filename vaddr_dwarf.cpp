#include "vaddr_dwarf.h"

static char null_name[] = "null_name";

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
    memset(&head, 0, sizeof(struct Vaddr_list));
    INIT_LIST_HEAD(&head.row);
    INIT_LIST_HEAD(&head.column);
    head.row_type = RowType_HEAD;

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

    /* 遍历dwarf */
    scan_dwarf(dbg, &head, &error);
    printf("\r\n\r\n");
    print_dwarf(dbg, &head, &error);
    printf("\r\n\r\n");

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

int Vaddr_Dwarf::get_dwarf_flag(Dwarf_Debug dbg, Dwarf_Die die, struct Vaddr_list *node, Dwarf_Error *error)
{
    Dwarf_Bool flag = 0;

    int res = DW_DLV_ERROR;

    if(node->row_type == RowType_VAR)
    {
        res = dwarf_die_flag(dbg, die, DW_AT_declaration, &flag, error);
        if (res == DW_DLV_ERROR) {
            printf("[%s-%s:%d] dwarf_die_flag() %s.\n", __FILE__, __func__, __LINE__, dwarf_errmsg(*error));
        } else if ((res == DW_DLV_NO_ENTRY) || (flag == 0)) {
            node->u.v.declaration = 0;
        } else if(flag == 1) {
            node->u.v.declaration = 1;
        }
    }

    return res;
}

int Vaddr_Dwarf::get_dwarf_bit(Dwarf_Debug dbg, Dwarf_Die die, struct Vaddr_list *node, Dwarf_Error *error)
{
    Dwarf_Unsigned ret_size = 0;
    Dwarf_Unsigned ret_offset = 0;
    Dwarf_Half attribute = 0;

    int res = 0;

    // 1、获取 var/mem bitsize信息
    res = dwarf_bitsize(die, &ret_size, error);
    if(res == DW_DLV_ERROR) {
        printf("[%s-%s:%d] dwarf_bitsize() %s.\n", __FILE__, __func__, __LINE__, dwarf_errmsg(*error));
        return res;
    } else if (res == DW_DLV_NO_ENTRY) {
        node->u.m.bit_size = 0;
    }
    else {
        node->u.m.bit_size = ret_size;
    }

    // 2、获取 var/mem bitoffset信息
    res = dwarf_bitoffset(die, &attribute, &ret_offset, error);
    if(res == DW_DLV_ERROR) {
        printf("[%s-%s:%d] dwarf_bitoffset() %s.\n", __FILE__, __func__, __LINE__, dwarf_errmsg(*error));
        return res;
    } else if (res == DW_DLV_NO_ENTRY) {
        node->u.m.bit_offset = 0;
    } else {
        node->u.m.bit_offset = ret_offset;
    }

    printf("bitsize:%u, bitoffset:%u\r\n", ret_size, ret_offset);

    return DW_DLV_OK;
}

int Vaddr_Dwarf::get_dwarf_loc(Dwarf_Debug dbg, Dwarf_Die die, struct Vaddr_list *node, Dwarf_Error *error)
{
    int res = DW_DLV_OK;

    if(node->row_type == RowType_VAR)
    {
        if(node->u.v.declaration == 0)
        {
            res = ASSERT2(dwarf_loc_info, dbg, die, node->u.v.operation, error);
        }
        else
        {
            memset((char *)node->u.v.operation, 0, sizeof(node->u.v.operation));
        }
    }
    else if(node->row_type == RowType_MEM)
    {
        res = ASSERT2(dwarf_loc_info, dbg, die, node->u.m.operation, error);
    }

    printf("operation:%d %d %d %d\r\n", node->u.m.operation[0], node->u.m.operation[1], 
        node->u.m.operation[2], node->u.m.operation[3]);

    return res;
}

int Vaddr_Dwarf::get_dwarf_name(Dwarf_Debug dbg, Dwarf_Die die, struct Vaddr_list *node, Dwarf_Error *error)
{
    char *diename = NULL;
    int res = 0;

    res = ASSERT2(dwarf_diename, die, &diename, error);
    if(res == DW_DLV_ERROR) {
        printf("[%s-%s:%d] dwarf_diename() %s.\n", __FILE__, __func__, __LINE__, dwarf_errmsg(*error));
        return res;
    } else if (res == DW_DLV_NO_ENTRY){
        diename = null_name;
    }

    memset(node->name, 0, sizeof(node->name));
    strncpy((char *)node->name, diename, (sizeof(node->name) - 1));

    return DW_DLV_OK;
}

int Vaddr_Dwarf::scan_type_dwarf(Dwarf_Debug dbg, Dwarf_Die parent_die, struct Vaddr_list *head, Dwarf_Error *error)
{
    Dwarf_Die mem_die = NULL;
    Dwarf_Die type_die = NULL;
    Dwarf_Die next_die = NULL;

    struct Vaddr_list *mem_head_node = NULL;
    int res = 0;

    // 2、获取一个mem
    res = dwarf_child(parent_die, &mem_die, error);
    if(res == DW_DLV_ERROR) {
        printf("[%s-%s:%d] dwarf_child() %s.\n", __FILE__, __func__, __LINE__, dwarf_errmsg(*error));
        goto RET;
    } else if (res == DW_DLV_NO_ENTRY){
        goto RET;
    }

    // 2.1、判断是不是mem die
    res = ASSERT2(dwarf_die_is_tag, dbg, mem_die, DW_TAG_member, error);
    if(res == DW_DLV_OK)
    {
        // 2.2、新建一个mem node
        struct Vaddr_list *mem_node = (struct Vaddr_list *)malloc(sizeof(struct Vaddr_list));
        memset(mem_node, 0, sizeof(struct Vaddr_list));

        // 2.3、初始化一个var node
        INIT_LIST_HEAD(&mem_node->row);
        INIT_LIST_HEAD(&mem_node->column);
        mem_node->row_type = RowType_MEM;
        get_dwarf_name(dbg, mem_die, mem_node, error);
        get_dwarf_loc(dbg, mem_die, mem_node, error);
        get_dwarf_bit(dbg, mem_die, mem_node, error);

        printf("mem name:%s\r\n", mem_node->name);

        // 2.4、往var list插入一个var node
        if(mem_head_node == NULL)
        {
            list_add(&mem_node->row, &head->row);
            mem_head_node = mem_node;
        }
        else
        {
            list_add_tail(&mem_node->column, &mem_head_node->column);
        }

        // 3、获取一个type
        res = ASSERT2(dwarf_die_basic_type2, dbg, mem_die, &type_die, 
            mem_node->u.m.deep, &mem_node->u.m.num, &mem_node->u.m.size, error);
        if(res != DW_DLV_OK) goto MEM;

        // 4、遍历type链表
        struct Vaddr_list *type_node = NULL;
        char *diename = NULL;
        res = dwarf_diename(type_die, &diename, error);
        if(res == DW_DLV_ERROR) {
            printf("[%s-%s:%d] dwarf_diename() %s.\n", __FILE__, __func__, __LINE__, dwarf_errmsg(*error));
            goto TYPE;
        } else if (res == DW_DLV_NO_ENTRY){
            diename = null_name;
        }
        
        // 4.1、遍历
        uint8_t exists = 0;
        if(strcmp((char *)head->name, diename) == 0)
        {
            type_node = head;
            exists = 1;
        }
        else
        {
            list_for_each_entry(type_node, &head->column, column) {
                if(strcmp((char *)type_node->name, diename) == 0)
                {
                    exists = 1;
                    break;
                }
            }
        }

        // 4.1、遍历存在
        if(exists)
        {
            mem_node->row.next = &type_node->row;
            printf("mem type name:%s\r\n", type_node->name);
        }
        // 4.1、遍历不存在
        else
        {
            // 3.1、新建一个type node
            type_node = (struct Vaddr_list *)malloc(sizeof(struct Vaddr_list));
            memset(type_node, 0, sizeof(struct Vaddr_list));

            // 3.2、初始化一个type node
            INIT_LIST_HEAD(&type_node->row);
            INIT_LIST_HEAD(&type_node->column);
            type_node->row_type = RowType_TYPE;
            memset(type_node->name, 0, sizeof(type_node->name));
            strncpy((char *)type_node->name, diename, (sizeof(type_node->name) - 1));

            printf("mem type name 2:%s\r\n", type_node->name);

            // 3.3、往type list插入一个type node
            list_add(&type_node->row, &mem_node->row);
            list_add_tail(&type_node->column, &head->column);

            // 5、递归type
            scan_type_dwarf(dbg, type_die, type_node, error);
        }

        dwarf_dealloc_die(type_die);
    }

    while(1) {
        res = dwarf_siblingof_b(dbg, mem_die, TRUE, &next_die, error);
        if(res == DW_DLV_ERROR) {
            printf("[%s-%s:%d] dwarf_siblingof_b() %s.\n", __FILE__, __func__, __LINE__, dwarf_errmsg(*error));
            goto MEM;
        } else if (res == DW_DLV_NO_ENTRY){
            break;
        }

        dwarf_dealloc_die(mem_die);
        mem_die = next_die;
        next_die = NULL;

        // 2.1、判断是不是mem die
        res = ASSERT2(dwarf_die_is_tag, dbg, mem_die, DW_TAG_member, error);
        if(res == DW_DLV_OK)
        {
            // 2.2、新建一个mem node
            struct Vaddr_list *mem_node = (struct Vaddr_list *)malloc(sizeof(struct Vaddr_list));
            memset(mem_node, 0, sizeof(struct Vaddr_list));

            // 2.3、初始化一个mem node
            INIT_LIST_HEAD(&mem_node->row);
            INIT_LIST_HEAD(&mem_node->column);
            mem_node->row_type = RowType_MEM;
            get_dwarf_name(dbg, mem_die, mem_node, error);
            get_dwarf_loc(dbg, mem_die, mem_node, error);
            get_dwarf_bit(dbg, mem_die, mem_node, error);

            printf("mem name 2:%s\r\n", mem_node->name);

            // 2.4、往mem list插入一个mem node
            if(mem_head_node == NULL)
            {
                list_add(&mem_node->row, &head->row);
                mem_head_node = mem_node;
            }
            else
            {
                list_add_tail(&mem_node->column, &mem_head_node->column);
            }

            // 3、获取一个type
            res = ASSERT2(dwarf_die_basic_type2, dbg, mem_die, &type_die, 
                mem_node->u.m.deep, &mem_node->u.m.num, (Dwarf_Unsigned *)&mem_node->u.m.size, error);
            if(res != DW_DLV_OK) goto MEM;

            // 4、遍历type链表
            struct Vaddr_list *type_node = NULL;
            char *diename = NULL;
            res = dwarf_diename(type_die, &diename, error);
            if(res == DW_DLV_ERROR) {
                printf("[%s-%s:%d] dwarf_diename() %s.\n", __FILE__, __func__, __LINE__, dwarf_errmsg(*error));
                goto TYPE;
            } else if (res == DW_DLV_NO_ENTRY){
                diename = null_name;
            }

            // 4.1、遍历
            uint8_t exists = 0;
            if(strcmp((char *)head->name, diename) == 0)
            {
                type_node = head;
                exists = 1;
            }
            else
            {
                list_for_each_entry(type_node, &head->column, column) {
                    if(strcmp((char *)type_node->name, diename) == 0)
                    {
                        exists = 1;
                        break;
                    }
                }
            }

            // 4.1、遍历存在
            if(exists)
            {
                mem_node->row.next = &type_node->row;
                printf("mem type name 3:%s\r\n", type_node->name);
            }
            // 4.1、遍历不存在
            else
            {
                // 3.1、新建一个type node
                type_node = (struct Vaddr_list *)malloc(sizeof(struct Vaddr_list));
                memset(type_node, 0, sizeof(struct Vaddr_list));

                // 3.2、初始化一个type node
                INIT_LIST_HEAD(&type_node->row);
                INIT_LIST_HEAD(&type_node->column);
                type_node->row_type = RowType_TYPE;
                memset(type_node->name, 0, sizeof(type_node->name));
                strncpy((char *)type_node->name, diename, (sizeof(type_node->name) - 1));

                printf("mem type name 4:%s\r\n", type_node->name);

                // 3.3、往type list插入一个type node
                list_add(&type_node->row, &mem_node->row);
                list_add_tail(&type_node->column, &head->column);

                // 5、递归type
                scan_type_dwarf(dbg, type_die, type_node, error);
            }

            dwarf_dealloc_die(type_die);
        }
    }

    dwarf_dealloc_die(mem_die);
    return DW_DLV_OK;

TYPE:
    dwarf_dealloc_die(type_die);
MEM:
    dwarf_dealloc_die(mem_die);
RET:
    return res;
}

int Vaddr_Dwarf::scan_dwarf(Dwarf_Debug dbg, struct Vaddr_list *head, Dwarf_Error *error)
{
    Dwarf_Die cu_die = NULL;
    Dwarf_Die var_die = NULL;
    Dwarf_Die type_die = NULL;
    Dwarf_Die next_die = NULL;

    struct Vaddr_list *unit_head_node = NULL;
    struct Vaddr_list *var_head_node = NULL;
    struct Vaddr_list *type_head_node = NULL;
    int res = 0;

    while(1) {
        // 1、获取一个unit
        res = dwarf_next_cu_die(dbg, &cu_die, error);
        if(res == DW_DLV_ERROR) {
            printf("[%s-%s:%d] dwarf_next_cu_die() %s.\n", __FILE__, __func__, __LINE__, dwarf_errmsg(*error));
            goto RET;
        } else if (res == DW_DLV_NO_ENTRY){
           goto RET;
        }

        // 1.1、新建一个unit node
        struct Vaddr_list *cu_node = (struct Vaddr_list *)malloc(sizeof(struct Vaddr_list));
        memset(cu_node, 0, sizeof(struct Vaddr_list));

        // 1.2、初始化一个unit node
        INIT_LIST_HEAD(&cu_node->row);
        INIT_LIST_HEAD(&cu_node->column);
        cu_node->row_type = RowType_UNIT;
        get_dwarf_name(dbg, cu_die, cu_node, error);

        printf("unit name:%s\r\n", cu_node->name);

        // 1.3、往unit list插入一个unit node
        if(unit_head_node == NULL)
        {
            list_add(&cu_node->row, &head->row);
            unit_head_node = cu_node;
        }
        else
        {
            list_add_tail(&cu_node->column, &unit_head_node->column);
        }
        
        // 2、获取一个var
        res = dwarf_child(cu_die, &var_die, error);
        if(res == DW_DLV_ERROR) {
            printf("[%s-%s:%d] dwarf_child() %s.\n", __FILE__, __func__, __LINE__, dwarf_errmsg(*error));
            goto CU;
        } else if (res == DW_DLV_NO_ENTRY){
            continue;
        }

        // 2.1、判断是不是var die
        res = ASSERT2(dwarf_die_is_tag, dbg, var_die, DW_TAG_variable, error);
        if(res == DW_DLV_OK)
        {
            // 2.2、新建一个var node
            struct Vaddr_list *var_node = (struct Vaddr_list *)malloc(sizeof(struct Vaddr_list));
            memset(var_node, 0, sizeof(struct Vaddr_list));

            // 2.3、初始化一个var node
            INIT_LIST_HEAD(&var_node->row);
            INIT_LIST_HEAD(&var_node->column);
            var_node->row_type = RowType_VAR;
            get_dwarf_name(dbg, var_die, var_node, error);
            get_dwarf_flag(dbg, var_die, var_node, error);
            get_dwarf_loc(dbg, var_die, var_node, error);

            printf("var name:%s\r\n", var_node->name);

            // 2.4、往var list插入一个var node
            var_head_node = NULL;
            if(var_head_node == NULL)
            {
                list_add(&var_node->row, &cu_node->row);
                var_head_node = var_node;
            }
            else
            {
                list_add_tail(&var_node->column, &var_head_node->column);
            }
            
            // 3、获取一个type
            res = ASSERT2(dwarf_die_basic_type2, dbg, var_die, &type_die, 
                var_node->u.v.deep, &var_node->u.v.num, (Dwarf_Unsigned *)&var_node->u.v.size, error);
            if(res != DW_DLV_OK) goto VAR;

            // 4、遍历type链表
            struct Vaddr_list *type_node = NULL;
            if(type_head_node == NULL)
            {
                // 3.1、新建一个type node
                type_node = (struct Vaddr_list *)malloc(sizeof(struct Vaddr_list));
                memset(type_node, 0, sizeof(struct Vaddr_list));

                // 3.2、初始化一个type node
                INIT_LIST_HEAD(&type_node->row);
                INIT_LIST_HEAD(&type_node->column);
                type_node->row_type = RowType_TYPE;
                get_dwarf_name(dbg, type_die, type_node, error);
                
                printf("type name:%s\r\n", type_node->name);

                // 3.3、往type list插入一个type node
                list_add(&type_node->row, &var_node->row);
                type_head_node = type_node;

                // 5、递归type
                scan_type_dwarf(dbg, type_die, type_node, error);
            }
            else
            {
                char *diename = NULL;
                res = dwarf_diename(type_die, &diename, error);
                if(res == DW_DLV_ERROR) {
                    printf("[%s-%s:%d] dwarf_diename() %s.\n", __FILE__, __func__, __LINE__, dwarf_errmsg(*error));
                    goto TYPE;
                } else if (res == DW_DLV_NO_ENTRY){
                    diename = null_name;
                }

                // 4.1、遍历
                uint8_t exists = 0;
                if(strcmp((char *)type_head_node->name, diename) == 0)
                {
                    type_node = type_head_node;
                    exists = 1;
                }
                else
                {
                    list_for_each_entry(type_node, &type_head_node->column, column) {
                        if(strcmp((char *)type_node->name, diename) == 0)
                        {
                            exists = 1;
                            break;
                        }
                    }
                }

                // 4.1、遍历存在
                if(exists)
                {
                    var_node->row.next = &type_node->row;
                    printf("type name 2:%s\r\n", type_node->name);
                }
                // 4.1、遍历不存在
                else
                {
                    // 3.1、新建一个type node
                    type_node = (struct Vaddr_list *)malloc(sizeof(struct Vaddr_list));
                    memset(type_node, 0, sizeof(struct Vaddr_list));

                    // 3.2、初始化一个type node
                    INIT_LIST_HEAD(&type_node->row);
                    INIT_LIST_HEAD(&type_node->column);
                    type_node->row_type = RowType_TYPE;
                    memset(type_node->name, 0, sizeof(type_node->name));
                    strncpy((char *)type_node->name, diename, (sizeof(type_node->name) - 1));

                    printf("type name 3:%s\r\n", type_node->name);

                    // 3.3、往type list插入一个type node
                    list_add(&type_node->row, &var_node->row);
                    list_add_tail(&type_node->column, &type_head_node->column);

                    // 5、递归type
                    scan_type_dwarf(dbg, type_die, type_node, error);
                }
            }

            dwarf_dealloc_die(type_die);
        }

        while(1)
        {
            // 2、获取一个var
            res = dwarf_siblingof_b(dbg, var_die, TRUE, &next_die, error);
            if(res == DW_DLV_ERROR) {
                printf("[%s-%s:%d] dwarf_siblingof_b() %s.\n", __FILE__, __func__, __LINE__, dwarf_errmsg(*error));
                goto VAR;
            } else if (res == DW_DLV_NO_ENTRY){
                break;
            }

            dwarf_dealloc_die(var_die);
            var_die = next_die;

            // 2.1、判断是不是var die
            res = ASSERT2(dwarf_die_is_tag, dbg, var_die, DW_TAG_variable, error);
            if(res == DW_DLV_OK)
            {
                // 2.2、新建一个var node
                struct Vaddr_list *var_node = (struct Vaddr_list *)malloc(sizeof(struct Vaddr_list));
                memset(var_node, 0, sizeof(struct Vaddr_list));

                // 2.3、初始化一个var node
                INIT_LIST_HEAD(&var_node->row);
                INIT_LIST_HEAD(&var_node->column);
                var_node->row_type = RowType_VAR;
                get_dwarf_name(dbg, var_die, var_node, error);
                get_dwarf_flag(dbg, var_die, var_node, error);
                get_dwarf_loc(dbg, var_die, var_node, error);

                printf("var name 2:%s\r\n", var_node->name);

                // 2.4、往var list插入一个var node
                if(var_head_node == NULL)
                {
                    list_add(&var_node->row, &cu_node->row);
                    var_head_node = var_node;
                }
                else
                {
                    list_add_tail(&var_node->column, &var_head_node->column);
                }
                
                // 3、获取一个type
                res = ASSERT2(dwarf_die_basic_type2, dbg, var_die, &type_die, 
                    var_node->u.v.deep, &var_node->u.v.num, (Dwarf_Unsigned *)&var_node->u.v.size, error);
                if(res != DW_DLV_OK) goto VAR;

                // 4、遍历type链表
                struct Vaddr_list *type_node = NULL;
                if(type_head_node == NULL)
                {
                    // 3.1、新建一个type node
                    type_node = (struct Vaddr_list *)malloc(sizeof(struct Vaddr_list));
                    memset(type_node, 0, sizeof(struct Vaddr_list));

                    // 3.2、初始化一个type node
                    INIT_LIST_HEAD(&type_node->row);
                    INIT_LIST_HEAD(&type_node->column);
                    type_node->row_type = RowType_TYPE;
                    get_dwarf_name(dbg, type_die, type_node, error);

                    printf("type name 4:%s\r\n", type_node->name);
                    
                    // 3.3、往type list插入一个type node
                    list_add(&type_node->row, &var_node->row);
                    type_head_node = type_node;

                    // 5、递归type
                    scan_type_dwarf(dbg, type_die, type_node, error);
                }
                else
                {
                    char *diename = NULL;
                    res = dwarf_diename(type_die, &diename, error);
                    if(res == DW_DLV_ERROR) {
                        printf("[%s-%s:%d] dwarf_diename() %s.\n", __FILE__, __func__, __LINE__, dwarf_errmsg(*error));
                        goto TYPE;
                    } else if (res == DW_DLV_NO_ENTRY){
                        diename = null_name;
                    }

                    // 4.1、遍历
                    uint8_t exists = 0;
                    if(strcmp((char *)type_head_node->name, diename) == 0)
                    {
                        type_node = type_head_node;
                        exists = 1;
                    }
                    else
                    {
                        list_for_each_entry(type_node, &type_head_node->column, column) {
                            if(strcmp((char *)type_node->name, diename) == 0)
                            {
                                exists = 1;
                                break;
                            }
                        }
                    }

                    // 4.1、遍历存在
                    if(exists)
                    {
                        var_node->row.next = &type_node->row;
                        printf("type name 5:%s\r\n", type_node->name);
                    }
                    // 4.1、遍历不存在
                    else
                    {
                        // 3.1、新建一个type node
                        type_node = (struct Vaddr_list *)malloc(sizeof(struct Vaddr_list));
                        memset(type_node, 0, sizeof(struct Vaddr_list));

                        // 3.2、初始化一个type node
                        INIT_LIST_HEAD(&type_node->row);
                        INIT_LIST_HEAD(&type_node->column);
                        type_node->row_type = RowType_TYPE;
                        memset(type_node->name, 0, sizeof(type_node->name));
                        strncpy((char *)type_node->name, diename, (sizeof(type_node->name) - 1));

                        printf("type name 6:%s\r\n", type_node->name);

                        // 3.3、往type list插入一个type node
                        list_add(&type_node->row, &var_node->row);
                        list_add_tail(&type_node->column, &type_head_node->column);
                        
                        // 5、递归type
                        scan_type_dwarf(dbg, type_die, type_node, error);
                    }
                }

                dwarf_dealloc_die(type_die);
            }
        }

        dwarf_dealloc_die(cu_die);
        dwarf_dealloc_die(var_die);
    }

    return DW_DLV_OK;

TYPE:
    dwarf_dealloc_die(type_die);
VAR:
    dwarf_dealloc_die(var_die);
CU:
    dwarf_dealloc_die(cu_die);
RET:
    return res;
}

int Vaddr_Dwarf::print_mem_dwarf(Dwarf_Debug dbg, struct Vaddr_list *head, Dwarf_Error *error, uint8_t *tab)
{
    if((head != NULL) && (head->row.next != NULL))
    {
        struct Vaddr_list *mem_head = list_entry(head->row.next, struct Vaddr_list, row);

        if(mem_head->row_type == RowType_MEM)
        {
            (*tab)++;

            for(int i = 0; i < (*tab); i++) printf("\t");
            printf("mem name:%s\r\n", mem_head->name);
            
            (*tab)++;
            print_type_dwarf(dbg, mem_head, error, tab);
            (*tab)--;

            struct Vaddr_list *mem_node = NULL;
            list_for_each_entry(mem_node, &mem_head->column, column) {
                for(int i = 0; i < (*tab); i++) printf("\t");
                printf("mem name:%s\r\n", mem_node->name);

                (*tab)++;
                print_type_dwarf(dbg, mem_node, error, tab);
                (*tab)--;
            }

            (*tab)--;
        }
        /*
        else
        {
            for(int i = 0; i < (*tab); i++) printf("\t");
            printf("WARRNING!!! NOT NEXT\r\n");
        }
        */
    }
    /*
    else
    {
        for(int i = 0; i < (*tab); i++) printf("\t");
        printf("WARRNING!!! NOT NEXT\r\n");
    }
    */

    return 0;
}

int Vaddr_Dwarf::print_type_dwarf(Dwarf_Debug dbg, struct Vaddr_list *head, Dwarf_Error *error, uint8_t *tab)
{
    if((head != NULL) && (head->row.next != NULL))
    {
        struct Vaddr_list *type_head = list_entry(head->row.next, struct Vaddr_list, row);

        if(type_head->row_type == RowType_TYPE)
        {
            for(int i = 0; i < (*tab); i++) printf("\t");
            printf("type name:%s\r\n", type_head->name);
            print_mem_dwarf(dbg, type_head, error, tab);
        }
        else
        {
            for(int i = 0; i < (*tab); i++) printf("\t");
            printf("WARRNING!!! TYPE ERR\r\n");
        }
    }

    return 0;
}

int Vaddr_Dwarf::print_var_dwarf(Dwarf_Debug dbg, struct Vaddr_list *head, Dwarf_Error *error)
{
    if((head != NULL) && (head->row.next != NULL))
    {
        struct Vaddr_list *var_head = list_entry(head->row.next, struct Vaddr_list, row);

        if(var_head->row_type == RowType_VAR)
        {
            uint8_t level = 2;

            printf("\tvar name:%s\r\n", var_head->name);
            print_type_dwarf(dbg, var_head, error, &level);

            struct Vaddr_list *var_node = NULL;
            list_for_each_entry(var_node, &var_head->column, column) {
                printf("\tvar name:%s\r\n", var_node->name);
                print_type_dwarf(dbg, var_node, error, &level);
            }
        }
    }

    return 0;
}

int Vaddr_Dwarf::print_dwarf(Dwarf_Debug dbg, struct Vaddr_list *head, Dwarf_Error *error)
{
    if((head != NULL) && (head->row.next != &head->row))
    {
        struct Vaddr_list *unit_head = list_entry(head->row.next, struct Vaddr_list, row);

        printf("unit name:%s\r\n", unit_head->name);
        print_var_dwarf(dbg, unit_head, error);

        struct Vaddr_list *unit_node = NULL;
        list_for_each_entry(unit_node, &unit_head->column, column) {
            printf("unit name:%s\r\n", unit_node->name);
            print_var_dwarf(dbg, unit_node, error);
        }
    }

    return 0;
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
