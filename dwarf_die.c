#include "dwarf_die.h"

static char dw_diename[] = "null";

int dwarf_get_machine(Dwarf_Debug dw_dbg, 
    Dwarf_Unsigned *machine, 
    Dwarf_Error *error) {
    Dwarf_Small    dw_ftype = 0;
    Dwarf_Small    dw_obj_pointersize = 0;
    Dwarf_Bool     dw_obj_is_big_endian = 0;
    Dwarf_Unsigned dw_obj_machine = 0;
    Dwarf_Unsigned dw_obj_flags = 0;
    Dwarf_Small    dw_path_source = 0;
    Dwarf_Unsigned dw_ub_offset = 0;
    Dwarf_Unsigned dw_ub_count = 0;
    Dwarf_Unsigned dw_ub_index = 0;
    Dwarf_Unsigned dw_comdat_groupnumber = 0;
    
    int res = dwarf_machine_architecture(
        dw_dbg,
        &dw_ftype,
        &dw_obj_pointersize,
        &dw_obj_is_big_endian,
        &dw_obj_machine,
        &dw_obj_flags,
        &dw_path_source,
        &dw_ub_offset,
        &dw_ub_count,
        &dw_ub_index,
        &dw_comdat_groupnumber
    );
    if (res != DW_DLV_OK) return res;

    (*machine) = dw_obj_machine;
    return DW_DLV_OK;
}

int dwarf_next_cu_die(Dwarf_Debug dw_dbg, 
    Dwarf_Die *cu_die, 
    Dwarf_Error *error) {
    /*
     * is_info = TRUE: 指针在 .debug_info 段中向后移动, 读取下一个 CU Header, 
     *  此时返回的 Header 结构描述该源码文件的编译信息(变量、函数定义、代码位置等全量信息).
     * is_info = FALSE: 指针在 .debug_types 段中向后移动, 读取下一个 TU Header, 
     *  此时返回的 Header 结构描述为了跨 CU 去重而抽离的类型信息(结构体、类等).
    */
    Dwarf_Bool is_info = TRUE;
    Dwarf_Unsigned cu_header_length = 0;
    Dwarf_Half     version_stamp = 0;
    Dwarf_Off      abbrev_offset = 0;
    Dwarf_Half     address_size  = 0;
    Dwarf_Half     length_size   = 0;
    Dwarf_Half     extension_size = 0;
    Dwarf_Sig8     type_signature = {0};
    Dwarf_Unsigned typeoffset     = 0;
    Dwarf_Unsigned next_cu_header_offset = 0;
    Dwarf_Half     header_cu_type = 0;

    while(1) {
        int res = dwarf_next_cu_header_d(
            dw_dbg,
            is_info, 
            &cu_header_length,  // 编译单元大小
            &version_stamp,     // dwarf版本号(2 to 5)
            &abbrev_offset,     // abbrev偏移
            &address_size,      // 地址值大小(4 or 8)
            &length_size,       // 偏移值大小(4)
            &extension_size,    // 拓展大小(64bitdwarf:4, 其他:0)
            &type_signature,    // 类型签名
            &typeoffset,        // 类型偏移
            &next_cu_header_offset, // 下一个编译单元偏移值
            &header_cu_type,    // 编译单元类型
            error
        );
        if (res != DW_DLV_OK) return res;
        if (header_cu_type == DW_UT_compile) break;
    }
    
    return dwarf_siblingof_b(dw_dbg, NULL, is_info, cu_die, error);
}

int dwarf_get_locAttr_operation(Dwarf_Debug dw_dbg, 
    Dwarf_Attribute attr, 
    int32_t *operation, 
    Dwarf_Error *error) {
    Dwarf_Loc_Head_c loclist_head = 0;
    Dwarf_Unsigned locentry_count = 0;
    
    int res = dwarf_get_loclist_c(attr, &loclist_head, &locentry_count, error);
    if (res != DW_DLV_OK) goto RET;

    for (int i = 0; i < locentry_count; ++i) {
        Dwarf_Small lle_value = 0;
        Dwarf_Unsigned rawval1 = 0;
        Dwarf_Unsigned rawval2 = 0;
        Dwarf_Bool debug_addr_unavailable = FALSE;
        Dwarf_Addr lopc = 0;
        Dwarf_Addr hipc = 0;
        Dwarf_Unsigned loclist_expr_op_count = 0;
        Dwarf_Locdesc_c locdesc_entry = 0;
        Dwarf_Small loclist_lkind = 0;
        Dwarf_Unsigned expression_offset = 0;
        Dwarf_Unsigned locdesc_offset = 0;

        res = dwarf_get_locdesc_entry_d(
            loclist_head, i,
            &lle_value,
            &rawval1,&rawval2,
            &debug_addr_unavailable,
            &lopc,&hipc,
            &loclist_expr_op_count,
            &locdesc_entry,
            &loclist_lkind,
            &expression_offset,
            &locdesc_offset,
            error);
        if (res != DW_DLV_OK) goto DEALLOC;

        for (int j = 0; j < loclist_expr_op_count; ++j) {
            Dwarf_Small op = 0;
            Dwarf_Unsigned opd1 = 0;
            Dwarf_Unsigned opd2 = 0;
            Dwarf_Unsigned opd3 = 0;
            Dwarf_Unsigned offsetforbranch = 0;

            res = dwarf_get_location_op_value_c(
                locdesc_entry, 
                j,
                &op, &opd1,&opd2,&opd3,
                &offsetforbranch,
                error);
            if (res != DW_DLV_OK) goto DEALLOC;

            operation[0] = op;
            operation[1] = opd1;
            operation[2] = opd2;
            operation[3] = opd3;
        }
    }

DEALLOC:
    dwarf_dealloc_loc_head_c(loclist_head);
RET:
    return res;
}

int dwarf_get_die_operation(Dwarf_Debug dw_dbg, 
    Dwarf_Die die, 
    Dwarf_Half attrnum,
    int32_t *operation, 
    Dwarf_Error *error) {
    Dwarf_Attribute ret_attr = NULL;

    int res = dwarf_attr(die, attrnum, &ret_attr, error);
    if (res == DW_DLV_ERROR) {
        goto RET;
    } else if (res == DW_DLV_NO_ENTRY) {
        if (attrnum == DW_AT_data_member_location) {
            Dwarf_Half version = 0;
            Dwarf_Bool is_info = 0;
            Dwarf_Bool is_dwo = 0;
            Dwarf_Half offset_size = 0;
            Dwarf_Half address_size = 0;
            Dwarf_Half extension_size = 0;
            Dwarf_Sig8 *signature = NULL;
            Dwarf_Off  offset_of_length = 0;
            Dwarf_Unsigned  total_byte_length = 0;

            res = dwarf_cu_header_basics(
                die,
                &version,
                &is_info,
                &is_dwo,
                &offset_size,
                &address_size,
                &extension_size,
                &signature,
                &offset_of_length,
                &total_byte_length,
                error);
            if (res != DW_DLV_OK) goto DEALLOC;
            else if ((version == 4) && (is_info == FALSE)) {
                operation[0] = DW_OP_plus_uconst;
                operation[1] = 0;
                operation[2] = 0;
                operation[3] = 0;
            } else res = DW_DLV_NO_ENTRY;
        }
    } else {
        res = dwarf_get_locAttr_operation(dw_dbg, ret_attr, operation, error);
    }

DEALLOC:
    dwarf_dealloc(dw_dbg, ret_attr, DW_DLA_ATTR);
RET:
    return res;
}

int dwarf_find_type_node(Dwarf_Debug dw_dbg, 
    st_dieNode_t *entry,
    st_dieNode_t *new_node,
    st_dieNode_t **exist_node,
    Dwarf_Error *error) {
    int res = 0;
    (*exist_node) = NULL;

    res = memcmp(&entry->un.type, &new_node->un.type, sizeof(new_node->un.type));
    if (res == 0) {
        (*exist_node) = entry;
    } else {
        st_dieNode_t *type_node = NULL;
        list_for_each_entry(type_node, &entry->column, column) {
            res = memcmp(&type_node->un.type, &new_node->un.type, sizeof(new_node->un.type));
            if (res == 0) {
                (*exist_node) = type_node;
            }
        }
    }

    return DW_DLV_OK;
}

int dwarf_get_die_type(Dwarf_Debug dw_dbg, 
    Dwarf_Die die, 
    Dwarf_Die *type_die, 
    Dwarf_Error *error) {
    Dwarf_Off offset = 0;
    Dwarf_Bool is_info = 0;

    int res = dwarf_dietype_offset(die, &offset, &is_info, error);
    if(res != DW_DLV_OK) return res;
    
    return dwarf_offdie_b(dw_dbg, offset, is_info, type_die, error);
}

int dwarf_get_array_info(Dwarf_Debug dw_dbg, 
    Dwarf_Die die, 
    uint32_t *dimension, 
    uint32_t *dimensionNum, 
    Dwarf_Error *error) {
    Dwarf_Die last_die = NULL;
    Dwarf_Die next_die = NULL;
    Dwarf_Attribute ret_attr = NULL;
    Dwarf_Unsigned ret_udata = 0;
    Dwarf_Half ret_tag = 0;
    int res = 0;
    (*dimensionNum) = 0;

    for (int i = 0; ; i++) {
        if(i == 0) {
            res = dwarf_child(die, &last_die, error);
            if(res != DW_DLV_OK) goto RET;
        } else {
            res = dwarf_siblingof_b(dw_dbg, last_die, TRUE, &next_die, error);
            if(res == DW_DLV_ERROR) {
                goto LAST;
            } else if(res == DW_DLV_NO_ENTRY) {
                if((*dimensionNum) != 0) res = DW_DLV_OK;
                goto LAST;
            }
            
            dwarf_dealloc_die(last_die);
            last_die = next_die;
        }

        dwarf_tag(last_die, &ret_tag, error);
        if(ret_tag == DW_TAG_subrange_type)
        {
            res = dwarf_attr(last_die, DW_AT_upper_bound, &ret_attr, error);
            if(res != DW_DLV_OK) goto LAST;

            res = dwarf_formudata(ret_attr, &ret_udata, error);
            if(res != DW_DLV_OK) goto LAST;

            dimension[(*dimensionNum)] = ret_udata + 1;
            (*dimensionNum)++;
        } else goto LAST;
    }

LAST:
    dwarf_dealloc_die(last_die);
RET:
    return res;
}

int dwarf_get_routine_info(Dwarf_Debug dw_dbg, 
    Dwarf_Die die, 
    st_dieNode_t **entry,
    Dwarf_Error *error) {
    Dwarf_Die ret_die = NULL;
    Dwarf_Die srt_die = NULL;
    st_dieNode_t *srt_entry = NULL;
    st_dieNode_t *type_entry = (*entry);
    int res = DW_DLV_OK;

    res = dwarf_get_die_type(dw_dbg, die, &ret_die, error);
    if (res == DW_DLV_ERROR) goto RET;
    else {
        // 1.2、新建一个mem node
        st_dieNode_t *srt_node = (st_dieNode_t *)malloc(sizeof(st_dieNode_t));

        // 1.3、初始化一个mem node
        RESET_LIST_HEAD(&srt_node->row);
        INIT_LIST_HEAD(&srt_node->column);
        srt_node->nodeType = NodeTyp_SRT;
        srt_node->un.srt.index = 0;

        list_add(&srt_node->row, &type_entry->row);
        srt_entry = srt_node;

        if (res == DW_DLV_OK) {
            st_dieNode_t *type_node = NULL;
            int dwarf_get_type_info(Dwarf_Debug dw_dbg, 
                Dwarf_Die die, 
                st_dieNode_t **entry,
                st_dieNode_t **node,
                Dwarf_Error *error);
            res = dwarf_get_type_info(dw_dbg, ret_die, &type_entry, &type_node, error);
            if(res != DW_DLV_OK) {
                printf("[%s-%s:%d] dwarf_get_type_info() %s.\n", __FILE__, __func__, __LINE__, dwarf_errmsg(*error));
                goto BACK;
            }

            list_add(&type_node->row, &srt_node->row);
        }
    }

    for (int i = 0, j = 1; ; i++)
    {
        // 1、获取一个mem
        if (i == 0) {
            res = dwarf_child(die, &srt_die, error);
            if (res == DW_DLV_ERROR) goto BACK;
            else if (res == DW_DLV_NO_ENTRY) {
                break;
            }
        } else {
            Dwarf_Die srt_die_tmp = NULL;
            
            res = dwarf_siblingof_b(dw_dbg, srt_die, TRUE, &srt_die_tmp, error);
            if (res == DW_DLV_ERROR) goto SRT;
            else if (res == DW_DLV_NO_ENTRY) {
                dwarf_dealloc_die(srt_die);
                break;
            }

            dwarf_dealloc_die(srt_die);
            srt_die = srt_die_tmp;
        }

        // 1.1、判断是不是mem_die
        Dwarf_Half ret_tag = 0;
        dwarf_tag(srt_die, &ret_tag, error);
        if (ret_tag == DW_TAG_formal_parameter) {
            // 1.2、新建一个mem node
            st_dieNode_t *srt_node = (st_dieNode_t *)malloc(sizeof(st_dieNode_t));

            // 1.3、初始化一个mem node
            RESET_LIST_HEAD(&srt_node->row);
            INIT_LIST_HEAD(&srt_node->column);
            srt_node->nodeType = NodeTyp_SRT;
            srt_node->un.srt.index = j++;

            // 1.7、往mem list插入一个mem node
            list_add_tail(&srt_node->column, &srt_entry->column);

            st_dieNode_t *type_node = NULL;
            int dwarf_get_type_info(Dwarf_Debug dw_dbg, 
                Dwarf_Die die, 
                st_dieNode_t **entry,
                st_dieNode_t **node,
                Dwarf_Error *error);
            res = dwarf_get_type_info(dw_dbg, srt_die, entry, &type_node, error);
            if(res != DW_DLV_OK) {
                printf("[%s-%s:%d] dwarf_get_type_info() %s.\n", __FILE__, __func__, __LINE__, dwarf_errmsg(*error));
                goto SRT;
            }
            //printf("dimension:%d %d %d %d %d %d\r\n", mem_node->un.mem.size, mem_node->un.mem.num,
            //    mem_node->un.mem.deep[0], mem_node->un.mem.deep[1], mem_node->un.mem.deep[2], mem_node->un.mem.deep[3]);

            list_add(&type_node->row, &srt_node->row);
        }
    }

    dwarf_dealloc_die(ret_die);
    (*entry) = type_entry;
    return DW_DLV_OK;

SRT:
    dwarf_dealloc_die(srt_die);
BACK:
    dwarf_dealloc_die(ret_die);
RET:
    return res;
}

int dwarf_get_member_info(Dwarf_Debug dw_dbg, 
    Dwarf_Die die, 
    st_dieNode_t **entry, 
    Dwarf_Error *error) {
    Dwarf_Die mem_die = NULL;
    st_dieNode_t *mem_entry = NULL;
    st_dieNode_t *type_entry = (*entry);
    int res = DW_DLV_OK;

    for (int i = 0; ; i++) {
        // 1、获取一个mem
        if (i == 0) {
            res = dwarf_child(die, &mem_die, error);
            if (res != DW_DLV_OK) goto RET;
        } else {
            Dwarf_Die mem_die_tmp = NULL;
            
            res = dwarf_siblingof_b(dw_dbg, mem_die, TRUE, &mem_die_tmp, error);
            if (res == DW_DLV_ERROR) goto MEM;
            else if (res == DW_DLV_NO_ENTRY) break;

            dwarf_dealloc_die(mem_die);
            mem_die = mem_die_tmp;
        }

        // 1.1、判断是不是mem_die
        Dwarf_Half ret_tag = 0;
        dwarf_tag(mem_die, &ret_tag, error);
        if(ret_tag == DW_TAG_member) {
            // 1.2、新建一个mem node
            st_dieNode_t *mem_node = (st_dieNode_t *)malloc(sizeof(st_dieNode_t));

            // 1.3、初始化一个mem node
            RESET_LIST_HEAD(&mem_node->row);
            INIT_LIST_HEAD(&mem_node->column);
            mem_node->nodeType = NodeTyp_MEM;

            // 1.4、获取mem_die的名字
            res = dwarf_diename(mem_die, &mem_node->un.mem.name, error);
            if(res == DW_DLV_ERROR) {
                printf("[%s-%s:%d] dwarf_diename() %s.\n", __FILE__, __func__, __LINE__, dwarf_errmsg(*error));
                goto MEM;
            } else if (res == DW_DLV_NO_ENTRY){
                mem_node->un.mem.name = dw_diename;
            }
            // printf("mem name:%s\r\n", mem_node->un.mem.name);

            // 1.5、获取mem_die的地址
            res = dwarf_get_die_operation(dw_dbg, mem_die, DW_AT_data_member_location, mem_node->un.mem.operation, error);
            if(res != DW_DLV_OK) {
                printf("[%s-%s:%d] dwarf_get_die_operation() %s.\n", __FILE__, __func__, __LINE__, dwarf_errmsg(*error));
                goto MEM;
            }
            // printf("operation:%d %d %d %d\r\n", mem_node->un.mem.operation[0], mem_node->un.mem.operation[1], 
            //     mem_node->un.mem.operation[2], mem_node->un.mem.operation[3]);

            // 1.6、获取mem_die的位
            // 1.6.1、获取 var/mem bitsize信息
            res = dwarf_bitsize(mem_die, &mem_node->un.mem.bit_size, error);
            if (res == DW_DLV_ERROR) goto MEM;
            else if (res == DW_DLV_NO_ENTRY) {
                mem_node->un.mem.bit_size = 0;
            }
            // 1.6.2、获取 var/mem bitoffset信息
            Dwarf_Half attrnum = 0;
            res = dwarf_bitoffset(mem_die, &attrnum, &mem_node->un.mem.bit_offset, error);
            if (res == DW_DLV_ERROR) goto MEM;
            else if (res == DW_DLV_NO_ENTRY) {
                mem_node->un.mem.bit_offset = 0;
            }
            // printf("bit:%d %d\r\n", mem_node->un.mem.bit_size, mem_node->un.mem.bit_offset);

            // 1.7、往mem list插入一个mem node
            if (mem_entry == NULL) {
                list_add(&mem_node->row, &type_entry->row);
                mem_entry = mem_node;
            } else {
                list_add_tail(&mem_node->column, &mem_entry->column);
            }

            // 2、获取mem_die的维度信息以及type
            st_dieNode_t *type_node = NULL;
            int dwarf_get_type_info(Dwarf_Debug dw_dbg, 
                Dwarf_Die die, 
                st_dieNode_t **entry,
                st_dieNode_t **node,
                Dwarf_Error *error);
            res = dwarf_get_type_info(dw_dbg, mem_die, &type_entry, &type_node, error);
            if(res != DW_DLV_OK) {
                printf("[%s-%s:%d] dwarf_get_type_info() %s.\n", __FILE__, __func__, __LINE__, dwarf_errmsg(*error));
                goto MEM;
            }
            //printf("dimension:%d %d %d %d %d %d\r\n", mem_node->un.mem.size, mem_node->un.mem.num,
            //    mem_node->un.mem.deep[0], mem_node->un.mem.deep[1], mem_node->un.mem.deep[2], mem_node->un.mem.deep[3]);

            list_add(&type_node->row, &mem_node->row);
        }
    }

    dwarf_dealloc_die(mem_die);
    (*entry) = type_entry;
    return DW_DLV_OK;

MEM:
    dwarf_dealloc_die(mem_die);
RET:
    return res;
}

int dwarf_get_type_info(Dwarf_Debug dw_dbg, 
    Dwarf_Die die, 
    st_dieNode_t **entry,
    st_dieNode_t **node,
    Dwarf_Error *error) {
    Dwarf_Die type_die = NULL;
    st_dieNode_t *type_entry = (*entry);
    int res = DW_DLV_OK;
    (*node) = NULL;

    // 1、新建一个type node
    st_dieNode_t *type_node = (st_dieNode_t *)malloc(sizeof(st_dieNode_t));
    memset(type_node, 0, sizeof(st_dieNode_t));

    // 2、初始化一个type node
    RESET_LIST_HEAD(&type_node->row);
    INIT_LIST_HEAD(&type_node->column);
    type_node->nodeType = NodeTyp_TYPE;
    
    for (int i = 0; ; i++) {
        // 1、获取 var/mem 基础类型
        if (i == 0) {
            res = dwarf_get_die_type(dw_dbg, die, &type_die, error);
            if (res != DW_DLV_OK) goto FREE;
        } else {
            Dwarf_Die type_die_tmp = NULL;
            
            res = dwarf_get_die_type(dw_dbg, type_die, &type_die_tmp, error);
            if (res != DW_DLV_OK) goto TYPE;

            dwarf_dealloc_die(type_die);
            type_die = type_die_tmp;
        }

        // 2、判断 var/mem 类型特殊处理
        Dwarf_Half ret_tag = 0;
        dwarf_tag(type_die, &ret_tag, error);
        if (ret_tag == DW_TAG_array_type) {
            // 2.1、获取 var/mem 数组维度
            res = dwarf_get_array_info(dw_dbg, type_die, type_node->un.type.dimension, 
                &type_node->un.type.dimensionNum, error);
            if(res != DW_DLV_OK) goto TYPE;
        } else if (ret_tag == DW_TAG_subroutine_type) {
            res = dwarf_diename(type_die, &type_node->un.type.name, error);
            if (res == DW_DLV_ERROR) {
                printf("[%s-%s:%d] dwarf_diename() %s.\n", __FILE__, __func__, __LINE__, dwarf_errmsg(*error));
                goto TYPE;
            } else if (res == DW_DLV_NO_ENTRY) {
                type_node->un.type.name = NULL;
            }
            if(type_node->un.type.pointer_type == TRUE) {
                Dwarf_Unsigned machine = EM_NONE;
                res = dwarf_get_machine(dw_dbg, &machine, error);
                if (res != DW_DLV_OK) goto TYPE;
                if (machine == EM_TI_C2000) {
                    type_node->un.type.byte_size = 2;
                } else {
                    type_node->un.type.byte_size = 4;
                }
            } else {
                res = dwarf_bytesize(type_die, &type_node->un.type.byte_size, error);
                if (res != DW_DLV_OK) goto TYPE;
            }
            if(type_entry == NULL) {
                type_entry = type_node;
                res = dwarf_get_routine_info(dw_dbg, type_die, &type_node, error);
                if(res != DW_DLV_OK) goto TYPE;
            } else {
                st_dieNode_t *exist_node = NULL;
                dwarf_find_type_node(dw_dbg, type_entry, type_node, &exist_node, error);
                if (exist_node == NULL) {
                    list_add_tail(&type_node->column, &type_entry->column);
                    res = dwarf_get_routine_info(dw_dbg, type_die, &type_node, error);
                    if(res != DW_DLV_OK) goto TYPE;
                } else {
                    free(type_node);
                    type_node = exist_node;
                }
            }
            break; // no subtype
        } else if ((ret_tag == DW_TAG_class_type) ||
            (ret_tag == DW_TAG_structure_type) ||
            (ret_tag == DW_TAG_union_type)) {
            res = dwarf_diename(type_die, &type_node->un.type.name, error);
            if (res == DW_DLV_ERROR) {
                printf("[%s-%s:%d] dwarf_diename() %s.\n", __FILE__, __func__, __LINE__, dwarf_errmsg(*error));
                goto TYPE;
            } else if (res == DW_DLV_NO_ENTRY) {
                type_node->un.type.name = NULL;
            }
            if(type_node->un.type.pointer_type == TRUE) {
                Dwarf_Unsigned machine = EM_NONE;
                res = dwarf_get_machine(dw_dbg, &machine, error);
                if (res != DW_DLV_OK) goto TYPE;
                if (machine == EM_TI_C2000) {
                    type_node->un.type.byte_size = 2;
                } else {
                    type_node->un.type.byte_size = 4;
                }
            } else {
                res = dwarf_bytesize(type_die, &type_node->un.type.byte_size, error);
                if (res != DW_DLV_OK) goto TYPE;
            }
            if(type_entry == NULL) {
                type_entry = type_node;
                res = dwarf_get_member_info(dw_dbg, type_die, &type_node, error);
                if(res != DW_DLV_OK) goto TYPE;
            } else {
                st_dieNode_t *exist_node = NULL;
                dwarf_find_type_node(dw_dbg, type_entry, type_node, &exist_node, error);
                if (exist_node == NULL) {
                    list_add_tail(&type_node->column, &type_entry->column);
                    res = dwarf_get_member_info(dw_dbg, type_die, &type_node, error);
                    if(res != DW_DLV_OK) goto TYPE;
                } else {
                    free(type_node);
                    type_node = exist_node;
                }
            }
            break; // no subtype
        } else if (ret_tag == DW_TAG_pointer_type) {
            type_node->un.type.pointer_type = TRUE;
        } else if (ret_tag == DW_TAG_reference_type) {
            type_node->un.type.reference_type = TRUE;
        } else if (ret_tag == DW_TAG_typedef) {
            res = dwarf_diename(type_die, &type_node->un.type.name_typedef, error);
            if (res == DW_DLV_ERROR) {
                printf("[%s-%s:%d] dwarf_diename() %s.\n", __FILE__, __func__, __LINE__, dwarf_errmsg(*error));
                goto TYPE;
            } else if (res == DW_DLV_NO_ENTRY) {
                type_node->un.type.name_typedef = NULL;
            }
        } else if (ret_tag == DW_TAG_base_type) {
            res = dwarf_diename(type_die, &type_node->un.type.name, error);
            if (res == DW_DLV_ERROR) {
                printf("[%s-%s:%d] dwarf_diename() %s.\n", __FILE__, __func__, __LINE__, dwarf_errmsg(*error));
                goto TYPE;
            } else if (res == DW_DLV_NO_ENTRY) {
                type_node->un.type.name = NULL;
            }
            if(type_node->un.type.pointer_type == TRUE) {
                Dwarf_Unsigned machine = EM_NONE;
                res = dwarf_get_machine(dw_dbg, &machine, error);
                if (res != DW_DLV_OK) goto TYPE;
                if (machine == EM_TI_C2000) {
                    type_node->un.type.byte_size = 2;
                } else {
                    type_node->un.type.byte_size = 4;
                }
            } else {
                res = dwarf_bytesize(type_die, &type_node->un.type.byte_size, error);
                if (res != DW_DLV_OK) goto TYPE;
            }
            if(type_entry == NULL) {
                type_entry = type_node;
            } else {
                st_dieNode_t *exist_node = NULL;
                dwarf_find_type_node(dw_dbg, type_entry, type_node, &exist_node, error);
                if (exist_node == NULL) {
                    list_add_tail(&type_node->column, &type_entry->column);
                } else {
                    free(type_node);
                    type_node = exist_node;
                }
            }
            break; // no subtype
        } 
    }

    dwarf_dealloc_die(type_die);
    (*node) = type_node;
    (*entry) = type_entry;
    return DW_DLV_OK;

TYPE:
    dwarf_dealloc_die(type_die);
FREE:
    free(type_node);
RET:
    return res;
}

int dwarf_die_init(Dwarf_Debug dw_dbg, 
    st_dieNode_t **entry, 
    Dwarf_Error *error) {
    Dwarf_Die cu_die = NULL;
    Dwarf_Die var_die = NULL;
    st_dieNode_t *cu_entry = NULL;
    st_dieNode_t *var_entry = NULL;
    st_dieNode_t *type_entry = NULL;
    int res = DW_DLV_OK;
	(*entry) = NULL; 

    while(1) {
        // 1、获取一个cu
        res = dwarf_next_cu_die(dw_dbg, &cu_die, error);
        if (res == DW_DLV_ERROR) {
            printf("[%s-%s:%d] dwarf_next_cu_die() %s.\n", __FILE__, __func__, __LINE__, dwarf_errmsg(*error));
            goto RET;
        } else if (res == DW_DLV_NO_ENTRY) {
            break;
        }

        // 1.1、新建一个cu node
        st_dieNode_t *cu_node = (st_dieNode_t *)malloc(sizeof(st_dieNode_t));

        // 1.2、初始化一个cu node
        RESET_LIST_HEAD(&cu_node->row);
        INIT_LIST_HEAD(&cu_node->column);
        cu_node->nodeType = NodeTyp_CU;

        // 1.4、往cu list插入一个cu node
        if (cu_entry == NULL) {
            cu_entry = cu_node;
        } else {
            list_add_tail(&cu_node->column, &cu_entry->column);
        }

        // 1.3、获取cu_die的名字
        res = dwarf_diename(cu_die, &cu_node->un.cu.name, error);
        if (res == DW_DLV_ERROR) {
            printf("[%s-%s:%d] dwarf_diename() %s.\n", __FILE__, __func__, __LINE__, dwarf_errmsg(*error));
            goto CU;
        } else if (res == DW_DLV_NO_ENTRY) {
            cu_node->un.cu.name = dw_diename;
        }
        // printf("cu name:%s\r\n", cu_node->un.cu.name);
        
        var_entry = NULL;
        for (int i = 0; ; i++) {
            // 2、获取一个var
            if (i == 0) {
                res = dwarf_child(cu_die, &var_die, error);
                if(res == DW_DLV_ERROR) {
                    printf("[%s-%s:%d] dwarf_child() %s.\n", __FILE__, __func__, __LINE__, dwarf_errmsg(*error));
                    goto CU;
                } else if (res == DW_DLV_NO_ENTRY){
                    break;
                }
            } else {
                Dwarf_Die var_die_tmp = NULL;

                res = dwarf_siblingof_c(var_die, &var_die_tmp, error);
                if (res == DW_DLV_ERROR) {
                    printf("[%s-%s:%d] dwarf_siblingof_b() %s.\n", __FILE__, __func__, __LINE__, dwarf_errmsg(*error));
                    goto VAR;
                } else if (res == DW_DLV_NO_ENTRY) {
                    dwarf_dealloc_die(var_die);
                    break;
                }

                dwarf_dealloc_die(var_die);
                var_die = var_die_tmp;
            }

            // 2.1、判断是不是var die
            Dwarf_Half ret_tag = 0;
            dwarf_tag(var_die, &ret_tag, error);
            if (ret_tag == DW_TAG_variable) {
                // 2.2、判断var_die是否是声明
				Dwarf_Attribute ret_attr = NULL;
                Dwarf_Bool ret_flag = false;
                
                res = dwarf_attr(var_die, DW_AT_declaration, &ret_attr, error);
                if (res == DW_DLV_ERROR) {
                    printf("[%s-%s:%d] dwarf_attr() %s.\n", __FILE__, __func__, __LINE__, dwarf_errmsg(*error));
                    goto VAR;
                } else if (res == DW_DLV_OK) {
                    res = dwarf_formflag(ret_attr, &ret_flag, error);
					dwarf_dealloc(dw_dbg, ret_attr, DW_DLA_ATTR);
                    if (res != DW_DLV_OK) {
                        printf("[%s-%s:%d] dwarf_formflag() %s.\n", __FILE__, __func__, __LINE__, dwarf_errmsg(*error));
                        goto VAR;
                    }
                }
                
                if ((res == DW_DLV_NO_ENTRY) || (ret_flag == false)) {
                    // 2.3、新建一个var node
                    st_dieNode_t *var_node = (st_dieNode_t *)malloc(sizeof(st_dieNode_t));

                    // 2.4、初始化一个var node
                    RESET_LIST_HEAD(&var_node->row);
                    INIT_LIST_HEAD(&var_node->column);
                    var_node->nodeType = NodeTyp_VAR;

                    // 2.5、往var list插入一个var node
                    if (var_entry == NULL) {
                        var_entry = var_node;
                        list_add(&var_node->row, &cu_node->row);
                    } else {
                        list_add_tail(&var_node->column, &var_entry->column);
                    }

                    // 2.6、获取var_die的名字
                    res = dwarf_diename(var_die, &var_node->un.var.name, error);
                    if (res == DW_DLV_ERROR) {
                        printf("[%s-%s:%d] dwarf_diename() %s.\n", __FILE__, __func__, __LINE__, dwarf_errmsg(*error));
                        goto VAR;
                    } else if (res == DW_DLV_NO_ENTRY) {
                        var_node->un.var.name = dw_diename;
                    }
                    // printf("var name:%s\r\n", var_node->un.var.name);
                    
                    // 2.7、获取var_die的地址
                    res = dwarf_get_die_operation(dw_dbg, var_die, DW_AT_location, 
                        var_node->un.var.operation, error);
                    if (res != DW_DLV_OK) {
                        printf("[%s-%s:%d] dwarf_get_die_operation() %s.\n", __FILE__, __func__, __LINE__, dwarf_errmsg(*error));
                        goto VAR;
                    }
                    // printf("operation:%d %d %d %d\r\n", var_node->un.var.operation[0], 
                    //     var_node->un.var.operation[1], var_node->un.var.operation[2], 
                    //     var_node->un.var.operation[3]);
                    
                    // 3、获取var_die的维度信息以及type
                    st_dieNode_t *type_node = NULL;
                    res = dwarf_get_type_info(dw_dbg, var_die, &type_entry, &type_node, error);
                    if (res != DW_DLV_OK) {
                        printf("[%s-%s:%d] dwarf_get_die_info() %s.\n", __FILE__, __func__, __LINE__, dwarf_errmsg(*error));
                        goto VAR;
                    }
                    // printf("dimension:%d %d %d %d %d %d\r\n", var_node->un.var.size, var_node->un.var.num, 
                    //      var_node->un.var.deep[0], var_node->un.var.deep[1], var_node->un.var.deep[2], var_node->un.var.deep[3]);
                
                    list_add(&type_node->row, &var_node->row);
                }
            }
        }

        dwarf_dealloc_die(cu_die);
    }

    (*entry) = cu_entry;
    return DW_DLV_OK;

VAR:
    dwarf_dealloc_die(var_die);
CU:
    dwarf_dealloc_die(cu_die);
RET:
    return res;
}

int dwarf_die_deinit(st_dieNode_t *entry) 
{
    if(entry != NULL)
    {
        
    }
}

int dwarf_print_type(Dwarf_Debug dw_dbg, 
    st_dieNode_t *entry, 
    Dwarf_Error *error,
    int level);

int dwarf_print_mem(Dwarf_Debug dbg, 
    st_dieNode_t *entry, 
    Dwarf_Error *error,
    int level)
{
    if(entry != NULL)
    {
        for(int i = 0; i < level; i++) printf("\t");
        printf("mem name:%s:(%d 0x%04x) <%d %d>", entry->un.mem.name,
            entry->un.mem.operation[0], entry->un.mem.operation[1],
            entry->un.mem.bit_size, entry->un.mem.bit_offset);
        if(entry->un.mem.num > 0)
        {
            printf(" [");
            for(int i = 0; i < entry->un.mem.num; i++) 
            {
                if(i == 0) printf("%d", entry->un.mem.deep[i]);
                else printf(" %d", entry->un.mem.deep[i]);
            }
            printf("]");
        }
        printf("\r\n");
        st_dieNode_t *type_entry = list_entry(entry->row.next, st_dieNode_t, row);
        dwarf_print_type(dbg, type_entry, error, level + 1);

        st_dieNode_t *mem_node = NULL;
        list_for_each_entry(mem_node, &entry->column, column) {
            for(int i = 0; i < level; i++) printf("\t");
            printf("mem name:%s:(%d 0x%04x) <%d %d>", mem_node->un.mem.name,
                mem_node->un.mem.operation[0], mem_node->un.mem.operation[1],
                mem_node->un.mem.bit_size, mem_node->un.mem.bit_offset);
            if(mem_node->un.mem.num > 0)
            {
                printf(" [");
                for(int i = 0; i < mem_node->un.mem.num; i++) 
                {
                    if(i == 0) printf("%d", mem_node->un.mem.deep[i]);
                    else printf(" %d", mem_node->un.mem.deep[i]);
                }
                printf("]");
            }
            printf("\r\n");
            st_dieNode_t *type_entry = list_entry(mem_node->row.next, st_dieNode_t, row);
            dwarf_print_type(dbg, type_entry, error, level + 1);
        }
    }

    return 0;
}

int dwarf_print_type(Dwarf_Debug dw_dbg, 
    st_dieNode_t *entry, 
    Dwarf_Error *error,
    int level)
{
    if(entry != NULL)
    {
        for(int i = 0; i < level; i++) printf("\t");
        printf("type name:%s\r\n", entry->un.type.name);
        st_dieNode_t *mem_entry = list_entry(entry->row.next, st_dieNode_t, row);
        dwarf_print_mem(dw_dbg, mem_entry, error, level + 1);
    }

    return 0;
}

int dwarf_print_var(Dwarf_Debug dw_dbg, 
    st_dieNode_t *entry, 
    Dwarf_Error *error)
{
    if(entry != NULL)
    {
        printf("\tvar name:%s:(%d 0x%04x)", entry->un.var.name, 
            entry->un.var.operation[0], entry->un.var.operation[1]);
        if(entry->un.var.num > 0)
        {
            printf(" [");
            for(int i = 0; i < entry->un.var.num; i++) 
            {
                if(i == 0) printf("%d", entry->un.var.deep[i]);
                else printf(" %d", entry->un.var.deep[i]);
            }
            printf("]");
        }
        printf("\r\n");
        st_dieNode_t *type_entry = list_entry(entry->row.next, st_dieNode_t, row);
        dwarf_print_type(dw_dbg, type_entry, error, 2);

        st_dieNode_t *var_node = NULL;
        list_for_each_entry(var_node, &entry->column, column) {
            printf("\tvar name:%s:(%d 0x%04x)", var_node->un.var.name, 
                var_node->un.var.operation[0], var_node->un.var.operation[1]);
            if(var_node->un.var.num > 0)
            {
                printf(" [");
                for(int i = 0; i < var_node->un.var.num; i++) 
                {
                    if(i == 0) printf("%d", var_node->un.var.deep[i]);
                    else printf(" %d", var_node->un.var.deep[i]);
                }
                printf("]");
            }
            printf("\r\n");
            st_dieNode_t *type_entry = list_entry(var_node->row.next, st_dieNode_t, row);
            dwarf_print_type(dw_dbg, type_entry, error, 2);
        }
    }

    return 0;
}

int dwarf_print_die(Dwarf_Debug dw_dbg, 
    st_dieNode_t *entry, 
    Dwarf_Error *error)
{
    if(entry != NULL)
    {
        printf("cu name:%s\r\n", entry->un.cu.name);
        st_dieNode_t *var_entry = list_entry(entry->row.next, st_dieNode_t, row);
        dwarf_print_var(dw_dbg, var_entry, error);

        st_dieNode_t *cu_node = NULL;
        list_for_each_entry(cu_node, &entry->column, column) {
            printf("cu name:%s\r\n", cu_node->un.cu.name);
            var_entry = list_entry(cu_node->row.next, st_dieNode_t, row);
            dwarf_print_var(dw_dbg, var_entry, error);
        }
    }

    return 0;
}
