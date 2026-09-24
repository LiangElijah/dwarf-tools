#include "dwarf_addr.h"

int dwarf_addr_cal(st_dieNode_t *entry, 
    st_str_t *strBuf, 
    st_addr_t *addrBuf) {
    st_dieNode_t *var_node = NULL;
    st_dieNode_t *mem_node = NULL;
    st_dieNode_t *type_node = NULL;
    uint32_t found = 0, address = 0;
    
    for (int n = 0; n < strBuf->segmentNum; n++) {
        if (n == 0) {
            found = 0;
            if (entry->row.next != NULL) {
                var_node = list_entry(entry->row.next, st_dieNode_t, row);
                if (strcmp((char *)var_node->un.var.name, strBuf->segment[n]) == 0) {
                    found = 1;
                } else {
                    st_dieNode_t *var_entry = var_node;
                    list_for_each_entry(var_node, &var_entry->column, column) {
                        if (strcmp((char *)var_node->un.var.name, strBuf->segment[n]) == 0) {
                            found = 1; break;
                        }
                    }
                }
            }
            if (found == 0) {
                st_dieNode_t *cu_node = NULL;
                list_for_each_entry(cu_node, &entry->column, column) {
                    if (cu_node->row.next != NULL) {
                        var_node = list_entry(cu_node->row.next, st_dieNode_t, row);
                        if (strcmp((char *)var_node->un.var.name, strBuf->segment[n]) == 0) {
                            found = 1; break;
                        } else {
                            st_dieNode_t *var_entry = var_node;
                            list_for_each_entry(var_node, &var_entry->column, column) {
                                if (strcmp((char *)var_node->un.var.name, strBuf->segment[n]) == 0) {
                                    found = 1; break;
                                }
                            }
                            if(found == 1) {
                                break;
                            } else {
                                printf("[%s-%s:%d] Unfound variable.\n", 
                                    __FILE__, __func__, __LINE__);
                                return AddrStatus_NOFOUND;
                            }
                        }
                    }
                }
            }
            
            if (var_node->un.var.operation[0] == DW_OP_addr) {
                address = var_node->un.var.operation[1];
            } else {
                printf("[%s-%s:%d] Unsupported Variable Operation (0x%02x).\n", 
                    __FILE__, __func__, __LINE__, 
                    var_node->un.var.operation[1]);
                return AddrStatus_UNSUPPORT;
            }

            if (var_node->row.next != NULL) {
                type_node = list_entry(var_node->row.next, st_dieNode_t, row);
            } else {
                printf("[%s-%s:%d] Unfound Variable Type.\n", 
                    __FILE__, __func__, __LINE__);
                return AddrStatus_NOTYPE;
            }
        } else /*if(n > 0)*/ {
            struct list_head *mem_head = NULL; found = 0;
            if (n == 1) {
                if((var_node->row.next != NULL) && (var_node->row.next->next != NULL)) {
                    mem_head = var_node->row.next->next;
                } else {
                    printf("[%s-%s:%d] Unfound Type or Member (%d).\n", 
                        __FILE__, __func__, __LINE__, n);
                    return AddrStatus_NOMEM;
                }
            } else {
                if((mem_head->next != NULL) && (mem_head->next->next != NULL)) {
                    mem_head = mem_head->next->next;
                } else {
                    printf("[%s-%s:%d] Unfound Type or Member (%d).\n", 
                        __FILE__, __func__, __LINE__, n);
                    return AddrStatus_NOMEM;
                }
            }
            mem_node = list_entry(mem_head, st_dieNode_t, row);
            if (strcmp((char *)mem_node->un.mem.name, strBuf->segment[n]) != 0) {
                st_dieNode_t *mem_entry = mem_node;
                list_for_each_entry(mem_node, &mem_entry->column, column) {
                    if (strcmp((char *)mem_node->un.mem.name, strBuf->segment[n]) == 0) {
                        found = 1; break;
                    }
                }
                if (found == 0) {
                    printf("[%s-%s:%d] Unfound Member (%d).\n", 
                        __FILE__, __func__, __LINE__, n);
                    return AddrStatus_NOFOUND;
                }
            }

            if (mem_node->un.mem.operation[0] == DW_OP_plus_uconst) {
                address += mem_node->un.mem.operation[1];
            } else {
                printf("[%s-%s:%d] Unsupported Operation (%d 0x%02x).\n", 
                    __FILE__, __func__, __LINE__, 
                    n, mem_node->un.mem.operation[1]);
                return AddrStatus_UNSUPPORT;
            }

            if (mem_node->row.next != NULL) {
                type_node = list_entry(mem_node->row.next, st_dieNode_t, row);
            } else {
                printf("[%s-%s:%d] Unfound Type (%d).\n", 
                        __FILE__, __func__, __LINE__, n);
                return AddrStatus_NOTYPE;
            }
        }

        if((strBuf->dimensionNum[n] > 0) && (strBuf->dimensionNum[n] <= type_node->un.type.dimensionNum)) {
            int offset = 0;
            if(((n + 1) < strBuf->segmentNum) && (strBuf->dimensionNum[n] != type_node->un.type.dimensionNum)) {
                printf("[%s-%s:%d] Illegal Varient Of Array (%d %d %d %d).\n", 
                    __FILE__, __func__, __LINE__, (n + 1), strBuf->segmentNum,
                    strBuf->dimensionNum[n], type_node->un.type.dimensionNum);
                return AddrStatus_ARRAY_LESS;
            }
            for(int d = 0; d < strBuf->dimensionNum[n]; d++) {
                int count = 0;
                if(strBuf->dimension[n][d] >= type_node->un.type.dimension[d]) {
                    printf("[%s-%s:%d] Array Out Of Range (%d %d).\n", 
                        __FILE__, __func__, __LINE__, 
                        strBuf->dimension[n][d], type_node->un.type.dimension[d]);
                    return AddrStatus_ARRAY_OUT;
                }
                for(int g = d + 1; g < type_node->un.type.dimensionNum; g++) {
                    if(count == 0) {
                        count = type_node->un.type.dimension[d];
                    } else {
                        count *= type_node->un.type.dimension[d];
                    }
                }
                if(count == 0) count = 1;
                offset += (count * strBuf->dimension[n][d]);
            }
            offset *= type_node->un.type.byte_size;
            address += offset;
        } else if(strBuf->dimensionNum[n] > 0) {
            printf("[%s-%s:%d] Illegal Array Target (%d %d).\n", 
                __FILE__, __func__, __LINE__, strBuf->dimensionNum[n], 
                type_node->un.type.dimensionNum);
            return AddrStatus_ARRAY_MORE;
        }
    }

    memset(addrBuf, 0, sizeof(st_addr_t));
    addrBuf->addr = address;
    addrBuf->strBuf = strBuf;
    addrBuf->type_node = type_node;
    if(mem_node != NULL) {
        addrBuf->bit_offset = mem_node->un.mem.bit_offset;
        addrBuf->bit_size = mem_node->un.mem.bit_size;
    } else {
        addrBuf->bit_offset = 0;
        addrBuf->bit_size = 0;
    }

    return AddrStatus_FOUND;
}

int dwarf_print_addr(st_addr_t *addrBuf) {
    st_dieNode_t *type_node = addrBuf->type_node;
    st_str_t *strBuf = addrBuf->strBuf;
    int dimensionNum_str = strBuf->dimensionNum[strBuf->segmentNum-1];
    int dimensionNum_type = type_node->un.type.dimensionNum;
    
    printf("ADDR: 0x%X | ", addrBuf->addr);
    
    if (type_node->un.type.type_tag == DW_TAG_subroutine_type) {
        printf("TYPE: ");
        st_dieNode_t *sub_entry = list_entry(type_node->row.next, st_dieNode_t, row);
        if(sub_entry->row.next == NULL) {
            printf("void (*)");
        } else {
            st_dieNode_t *ret_entry = list_entry(sub_entry->row.next, st_dieNode_t, row);
            if(ret_entry->un.type.name == NULL) {
                printf("%s", ret_entry->un.type.name_typedef);
            } else {
                if (ret_entry->un.type.type_tag == DW_TAG_structure_type) printf("struct %s", ret_entry->un.type.name);
                else if (ret_entry->un.type.type_tag == DW_TAG_union_type) printf("union %s", ret_entry->un.type.name);
                else printf("%s", ret_entry->un.type.name);
            }
            printf(" (*%s)");
        }
        st_dieNode_t *sub_node = NULL; int i = 0; printf("(");
        list_for_each_entry(sub_node, &sub_entry->column, column) {
            i++; if(i > 1) printf(", ");

            st_dieNode_t *param_entry = list_entry(sub_node->row.next, st_dieNode_t, row);
            if (param_entry->un.type.name == NULL) {
                printf("%s", param_entry->un.type.name_typedef);
            } else {
                if (param_entry->un.type.type_tag == DW_TAG_structure_type) printf("struct %s", param_entry->un.type.name);
                else if (param_entry->un.type.type_tag == DW_TAG_union_type) printf("union %s", param_entry->un.type.name);
                else printf("%s", param_entry->un.type.name);
            }
        }
        if (i == 0) printf("void)");
        else printf(")");
    } else {
        if (type_node->un.type.name == NULL) {
            printf("TYPE: %s", type_node->un.type.name_typedef);
        } else {
            if (type_node->un.type.type_tag == DW_TAG_class_type) printf("TYPE: class %s", type_node->un.type.name);
            else if (type_node->un.type.type_tag == DW_TAG_structure_type) printf("TYPE: struct %s", type_node->un.type.name);
            else if (type_node->un.type.type_tag == DW_TAG_union_type) printf("TYPE: union %s", type_node->un.type.name);
            else printf("TYPE: %s", type_node->un.type.name);
        }
        if (type_node->un.type.pointer_type == TRUE) {
            printf(" *");
        }
    }

    if (dimensionNum_str < dimensionNum_type) {
        for (int d = dimensionNum_str; d < dimensionNum_type; d++) {
            printf("[%d]", type_node->un.type.dimension[d]);
        }
    } else if (addrBuf->bit_size > 0) {
        printf(" | ");
        if (addrBuf->bit_size == 1) {
            printf("Bit:%d(%d)", 15-addrBuf->bit_offset, addrBuf->bit_size);
        } else {
            printf("Bit:%d-%d(%d)", 15 - addrBuf->bit_offset - addrBuf->bit_size + 1, 
                15 - addrBuf->bit_offset, addrBuf->bit_size);
        }
    }

    printf("\r\n");
    return 0;
}