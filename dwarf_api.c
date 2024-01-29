#include "dwarf_api.h"

/* DIE API */
int dwarf_die_udata(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Half attrnum, Dwarf_Unsigned *ret_udata, Dwarf_Error *error)
{
    Dwarf_Attribute ret_attr = NULL;
    int res = 0;
    
    res = dwarf_attr(die, attrnum, &ret_attr, error);
    if (res != DW_DLV_OK) return res;

    res = dwarf_formudata(ret_attr, ret_udata, error);
    dwarf_dealloc(dbg, ret_attr, DW_DLA_ATTR);
    return res;
}

int dwarf_die_sdata(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Half attrnum, Dwarf_Signed *ret_sdata, Dwarf_Error *error)
{
    Dwarf_Attribute ret_attr = NULL;
    int res = 0;
    
    res = dwarf_attr(die, attrnum, &ret_attr, error);
    if (res != DW_DLV_OK) return res;

    res = dwarf_formsdata(ret_attr, ret_sdata, error);
    dwarf_dealloc(dbg, ret_attr, DW_DLA_ATTR);
    return res;
}

int dwarf_die_string(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Half attrnum, char **string, Dwarf_Error *error)
{
    Dwarf_Attribute ret_attr = NULL;
    int res = 0;
    
    res = dwarf_attr(die, attrnum, &ret_attr, error);
    if (res != DW_DLV_OK) return res;

    res = dwarf_formstring(ret_attr, string, error);
    dwarf_dealloc(dbg, ret_attr, DW_DLA_ATTR);
    return res;
}

int dwarf_die_flag(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Half attrnum, Dwarf_Bool *ret_flag, Dwarf_Error *error)
{
    Dwarf_Attribute ret_attr = NULL;
    int res = 0;
    
    res = dwarf_attr(die, attrnum, &ret_attr, error);
    if(res != DW_DLV_OK) return res;

    res = dwarf_formflag(ret_attr, ret_flag, error);
    dwarf_dealloc(dbg, ret_attr, DW_DLA_ATTR);
    return res;
}

int dwarf_die_type(Dwarf_Debug dbg, Dwarf_Die main_die, Dwarf_Die *type_die, Dwarf_Error *error)
{
    Dwarf_Off offset = 0;
    Dwarf_Bool is_info = 0;

    int res = 0;

    res = dwarf_dietype_offset(main_die, &offset, &is_info, error);
    if(res != DW_DLV_OK) return res;

    res = dwarf_offdie_b(dbg, offset, is_info, type_die, error);
    if(res != DW_DLV_OK) return res;

    return DW_DLV_OK;
}

int dwarf_die_basic_type(Dwarf_Debug dbg, Dwarf_Die main_die, Dwarf_Die *type_die, Dwarf_Error *error)
{
    Dwarf_Die type_die_tmp = NULL;
    Dwarf_Die type_die_tmp2 = NULL;

    int res = 0;

    res = ASSERT2(dwarf_die_type, dbg, main_die, &type_die_tmp, error);
    if(res != DW_DLV_OK) return res;

    while(1) {
        res = dwarf_die_type(dbg, type_die_tmp, &type_die_tmp2, error);
        if(res == DW_DLV_ERROR) {
            printf("[%s-%s:%d] dwarf_die_type() %s.\n", __FILE__, __func__, __LINE__, dwarf_errmsg(*error));
            return res;
        }
        else if(res == DW_DLV_NO_ENTRY) break;

        dwarf_dealloc_die(type_die_tmp);
        type_die_tmp = type_die_tmp2;
    }

    *type_die = type_die_tmp;
    return DW_DLV_OK;
}

/* DIE API 2 */
int dwarf_die_is_tag(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Half tag, Dwarf_Error *error)
{
    Dwarf_Half ret_tag = 0;

    int res = ASSERT2(dwarf_tag, die, &ret_tag, error);
    if(res != DW_DLV_OK) return res;

    if(ret_tag == tag) {
        return DW_DLV_OK;
    } else {
        return DW_DLV_NOT_CMP;
    }
}

int dwarf_die_is_array(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Error *error)
{
    Dwarf_Off offset = 0;
    Dwarf_Bool is_info = 0;
    Dwarf_Die type_die = NULL;

    int res = 0;

    res = ASSERT2(dwarf_dietype_offset, die, &offset, &is_info, error);
    if(res != DW_DLV_OK) return res;

    res = ASSERT2(dwarf_offdie_b, dbg, offset, is_info, &type_die, error);
    if(res != DW_DLV_OK) return res;

    res = ASSERT2(dwarf_die_is_tag, dbg, type_die, DW_TAG_array_type, error);
    return res;
}

/* ATTR API */
int dwarf_attr_loc(Dwarf_Debug dbg, Dwarf_Attribute attr, int32_t *operation, Dwarf_Error *error)
{
    Dwarf_Loc_Head_c loclist_head = 0;
    Dwarf_Unsigned locentry_count = 0;
    
    int res = 0;

    res = ASSERT2(dwarf_get_loclist_c, attr, &loclist_head, &locentry_count, error);
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

        res = ASSERT2(
            dwarf_get_locdesc_entry_d,
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
        if (res != DW_DLV_OK) goto ERROR;

        for (int j = 0; j < loclist_expr_op_count; ++j) {
            Dwarf_Small op = 0;
            Dwarf_Unsigned opd1 = 0;
            Dwarf_Unsigned opd2 = 0;
            Dwarf_Unsigned opd3 = 0;
            Dwarf_Unsigned offsetforbranch = 0;

            res = ASSERT2(
                dwarf_get_location_op_value_c,
                locdesc_entry, j,
                &op, &opd1,&opd2,&opd3,
                &offsetforbranch,
                error);
            if (res != DW_DLV_OK) goto ERROR;

            operation[0] = op;
            operation[1] = opd1;
            operation[2] = opd2;
            operation[3] = opd3;
        }
    }

ERROR:
    dwarf_dealloc_loc_head_c(loclist_head);
RET:
    return res;
}

/* ATTR API 2 */
int dwarf_attr_is_locexpr(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Attribute attr, Dwarf_Error *error)
{
    Dwarf_Half form = 0;
    Dwarf_Half attrnum = 0;
    Dwarf_Half version = 0;
    Dwarf_Half offset_size = 0;

    int res = 0;

    res = ASSERT2(dwarf_whatform, attr, &form, error);
    if(res != DW_DLV_OK) return res;

    res = ASSERT2(dwarf_whatattr, attr, &attrnum, error);
    if(res != DW_DLV_OK) return res;
    
    res = ASSERT2(dwarf_get_version_of_die, die, &version, &offset_size);
    if(res != DW_DLV_OK) return res;
    
    res = ASSERT2(dwarf_get_form_class, version, attrnum, offset_size, form);
    if(res == DW_FORM_CLASS_EXPRLOC) {
        return DW_DLV_OK;
    } else {
        return DW_DLV_NOT_CMP;
    }
}

/* COMMON API */
void dwarf_dealloc_list(Dwarf_Debug dbg, Dwarf_Attribute *attrbuf, Dwarf_Signed attrcount)
{
    for (int i = 0; i < attrcount; i++) {
        dwarf_dealloc_attribute(attrbuf[i]);
    }
    dwarf_dealloc(dbg, attrbuf, DW_DLA_LIST);
}

int dwarf_next_cu_die(Dwarf_Debug dbg, Dwarf_Die *cu_die, Dwarf_Error *error)
{
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

    int res = 0;
    
    res = ASSERT2(
        dwarf_next_cu_header_d,
        dbg,
        TRUE,
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
    if(res != DW_DLV_OK) return res;

    res = ASSERT2(dwarf_siblingof_b, dbg, NULL, TRUE, cu_die, error);
    return res;
}

int dwarf_array_info(Dwarf_Debug dbg, Dwarf_Die die, uint32_t *array, uint32_t *num, Dwarf_Error *error)
{
    Dwarf_Unsigned ret_udata = 0;
    Dwarf_Die type_die = NULL;
    Dwarf_Die last_die = NULL;
    Dwarf_Die next_die = NULL;

    int res = 0;
    (*num) = 0;

    res = ASSERT2(dwarf_die_is_array, dbg, die, error);
    if(res == DW_DLV_OK) {
        res = ASSERT2(dwarf_die_type, dbg, die, &type_die, error);
        if(res != DW_DLV_OK) goto RET;

        res = ASSERT2(dwarf_child, type_die, &last_die, error);
        if(res != DW_DLV_OK) goto TYPE;

        res = ASSERT2(dwarf_die_is_tag, dbg, last_die, DW_TAG_subrange_type, error);
        if(res == DW_DLV_OK) {
            res = ASSERT2(dwarf_die_udata, dbg, last_die, DW_AT_upper_bound, &ret_udata, error);
            if(res != DW_DLV_OK) goto LAST;

            array[(*num)] = ret_udata + 1;
            (*num)++;
        } else if(res != DW_DLV_NOT_CMP) {
            goto LAST;
        }

        while(1) {
            res = dwarf_siblingof_b(dbg, last_die, TRUE, &next_die, error);
            if(res == DW_DLV_ERROR) {
                goto LAST;
            } else if(res == DW_DLV_NO_ENTRY) {
                if((*num) != 0) res = DW_DLV_OK;
                goto LAST;
            }
            
            dwarf_dealloc_die(last_die);
            last_die = next_die;

            res = ASSERT2(dwarf_die_is_tag, dbg, last_die, DW_TAG_subrange_type, error);
            if(res == DW_DLV_OK) {
                res = ASSERT2(dwarf_die_udata, dbg, last_die, DW_AT_upper_bound, &ret_udata, error);
                if(res != DW_DLV_OK) goto LAST;

                array[(*num)] = ret_udata + 1;
                (*num)++;
            } else if(res != DW_DLV_NOT_CMP) {
                goto LAST;
            }
        }
    }

LAST:
    dwarf_dealloc_die(last_die);
TYPE:
    dwarf_dealloc_die(type_die);
RET:
    return res;
}

int dwarf_loc_info(Dwarf_Debug dbg, Dwarf_Die die, int32_t *operation, Dwarf_Error *error)
{
    Dwarf_Attribute *attrbuf = 0;
    Dwarf_Signed attrcount = 0;

    int res = 0, i = 0;

    res = ASSERT2(dwarf_attrlist, die, &attrbuf, &attrcount, error);
    if(res != DW_DLV_OK) goto RET;

    for (i = 0; i < attrcount; i++) {
        res = ASSERT2(dwarf_attr_is_locexpr, dbg, die, attrbuf[i], error);
        if (res == DW_DLV_OK) {
            break;
        } else if((res != DW_DLV_NOT_CMP)) {
            goto DEALLOC;
        }
    }

    if(i == attrcount)
    {
        Dwarf_Half version = 0;
        Dwarf_Bool is_info = 0;
        Dwarf_Bool is_dwo = 0;
        Dwarf_Half offset_size = 0;
        Dwarf_Half address_size = 0;
        Dwarf_Half extension_size = 0;
        Dwarf_Sig8 *signature = NULL;
        Dwarf_Off  offset_of_length = 0;
        Dwarf_Unsigned  total_byte_length = 0;

        res = ASSERT2(
            dwarf_cu_header_basics,
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
        if(res != DW_DLV_OK) goto DEALLOC;

        res = ASSERT2(dwarf_die_is_tag, dbg, die, DW_TAG_member, error);
        if((res != DW_DLV_OK) && (res != DW_DLV_NOT_CMP)) goto DEALLOC;

        if((res == DW_DLV_OK) && (version == 4) && (is_info == FALSE)) {
            operation[0] = DW_OP_plus_uconst;
            operation[1] = 0;
            operation[2] = 0;
            operation[3] = 0;
        } else {
            printf("[%s-%s:%d] No locexpr.\n", __FILE__, __func__, __LINE__);
            res = DW_DLV_NO_ENTRY;
        }
    } else {
        res = ASSERT2(dwarf_attr_loc, dbg, attrbuf[i], operation, error);
    }

DEALLOC:
    dwarf_dealloc_list(dbg, attrbuf, attrcount);
RET:
    return res;
}