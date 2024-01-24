#include "vaddr_string.h"

Vaddr_String::Vaddr_String(const char *vstr)
{
    parse(vstr);
}

Vaddr_String::Vaddr_String()
{
    is_ready = 0;
}

Vaddr_String::~Vaddr_String()
{

}

int Vaddr_String::parse(const char *vstr)
{
    is_ready = 0;
    is_analyzed = 0;
    is_caled = 0;

    if(vstr == NULL) return -1;

    for(int index = 0, count = 0; index < (strlen(vstr) + 1); index++) {
        if(vstr[index] == '\0') {   // 字符串以 '\0' 结尾
            if(count == 0) {        // 以 . 结尾或空字符串
                printf("[%s-%s:%d] End With \'.\' or Empty String. (%s)\n", __FILE__, __func__, __LINE__, vstr);
                return -1;
            }

            vstr_part_num++;
            break;
        } else if(vstr[index] != '.') { // 成员字符
            if(vstr[index] == '[') {    // 数组起始符
                if(count == 0) {        // 变量以 [ 开头
                    printf("[%s-%s:%d] Begin With \'[\'. (%s)\n", __FILE__, __func__, __LINE__, vstr);
                    return -1;
                }

                char number[16] = {0};
                int pos = 0;

                while(vstr[++index] != ']') {   // 数组结束符
                    if(vstr[index] == '\0') {   // 无 ']' 结尾 
                        printf("[%s-%s:%d] No \']\'. (%s)\n", __FILE__, __func__, __LINE__, vstr);
                        return -1;
                    } else if((vstr[index] > 57) || ((vstr[index] < 48))) { // 非数字
                        printf("[%s-%s:%d] Not Num. (%s)\n", __FILE__, __func__, __LINE__, vstr);
                        return -1;
                    }

                    number[pos] = vstr[index];
                    pos++;
                }

                if(pos == 0) {  // '[' 和 ']' 之间无数字 
                    printf("[%s-%s:%d] No Num. (%s)\n", __FILE__, __func__, __LINE__, vstr);
                    return -1;
                }
                dms[vstr_part_num].deep[dms[vstr_part_num].num] = atoi(number);
                dms[vstr_part_num].num++;
            } else {    // 非数组成员
                if(dms[vstr_part_num].num != 0) {   // 数组后接成员字符
                    printf("[%s-%s:%d] Unknown Format. (%s)\n", __FILE__, __func__, __LINE__, vstr);
                    return -1;
                }

                vstr_part[vstr_part_num][count] = vstr[index];
                count++;
            }
        } else {    // 成员间隔符
            if(count == 0) {    // 以 . 开头或只有一个 .
                printf("[%s-%s:%d] Begin With \'.\' or Only \'.\'. (%s)\n", __FILE__, __func__, __LINE__, vstr);
                return -1;
            }

            vstr_part_num++;
            count = 0;
        }
    }

    memcpy(this->vstr, vstr, strlen(vstr) + 1);

    is_ready = 1;
    return 0;
}

int Vaddr_String::ready()
{
    return is_ready;
}

int Vaddr_String::analyzed()
{
    return is_analyzed;
}

int Vaddr_String::caled()
{
    return is_caled;
}

int Vaddr_String::calculate()
{
    if(this->analyzed() != 1) {
        printf("[%s-%s:%d] Something not ready.\n", __FILE__, __func__, __LINE__);
        return -1;
    }
    
    objaddr = 0;
    for(int n = 0; n < vstr_part_num; n++) {
        if((operation[n][0] == DW_OP_addr) && (n == 0)) {
            objaddr = operation[n][1];
        } else if((operation[n][0] == DW_OP_plus_uconst) && (n > 0)) {
            objaddr += operation[n][1];
        } else {
            printf("[%s-%s:%d] Unsupported DW_OP Type (%d 0x%02x).\n", 
                __FILE__, __func__, __LINE__, 
                n, operation[n][0]);
            return -1;
        }

        if((dms[n].num > 0) && (dms[n].num <= dms_file[n].num)) {
            int offset = 0;
            if(((n + 1) < vstr_part_num) && (dms[n].num != dms_file[n].num)) {
                printf("[%s-%s:%d] Illegal Varient Of Array (%d %d %d %d).\n", 
                    __FILE__, __func__, __LINE__, 
                    (n + 1), vstr_part_num,
                    dms[n].num, dms_file[n].num);
                return -1;
            }
            for(int d = 0; d < dms[n].num; d++) {
                int count = 0;
                if(dms[n].deep[d] >= dms_file[n].deep[d]) {
                    printf("[%s-%s:%d] Array Out Of Range (%d %d).\n", 
                        __FILE__, __func__, __LINE__, 
                        dms[n].deep[d], dms_file[n].deep[d]);
                    return -1;
                }
                for(int g = d + 1; g < dms_file[n].num; g++) {
                    if(count == 0) {
                        count = dms_file[n].deep[g];
                    } else {
                        count *= dms_file[n].deep[g];
                    }
                }
                if(count == 0) count = 1;
                offset += (count * dms[n].deep[d]);
            }
            offset *= dms_file[n].size;
            objaddr += offset;
        } else if(dms[n].num > 0) {
            printf("[%s-%s:%d] Illegal Array Target (%d %d).\n", 
                __FILE__, __func__, __LINE__, 
                dms[n].num, dms_file[n].num);
            return -1;
        }
    }

    is_caled = 1;
    return is_caled;
}

void Vaddr_String::print_cal()
{
    if(this->caled() != 1) {
        printf("[%s-%s:%d] Something not calculate.\n", __FILE__, __func__, __LINE__);
        return;
    }
    
    if(dms_file[vstr_part_num-1].num != 0)
    {
        printf("ADDR:0x%X Type:%s", objaddr, vtype);
        for(int d = dms[vstr_part_num-1].num; d < dms_file[vstr_part_num-1].num; d++)
        {
            printf("[%d]", dms_file[vstr_part_num-1].deep[d]);
        }
        printf("\n");
    }
    else if(bitsize > 0)
    {
        if(bitsize == 1)
        {
            printf("Addr:0x%X | Type:%s | Bit:%d(%d)\n", objaddr, vtype,
                15-bitoffset, bitsize);
        }
        else
        {
            printf("Addr:0x%X | Type:%s | Bit:%d-%d(%d)\n", objaddr, vtype, 
                15 - bitoffset, 15 - bitoffset - bitsize + 1, bitsize);
        }
    }
    else
    {
        printf("ADDR:0x%X | Type:%s\n", objaddr, vtype);
    }
}

void Vaddr_String::print_dms()
{
    if(this->ready() != 1) {
        printf("[%s-%s:%d] Something not ready.\n", __FILE__, __func__, __LINE__);
        return;
    }

    printf("Variant Got:\n");
    for(int n = 0; n < vstr_part_num; n++) {
        printf("[%d] %s", n, vstr_part[n]);
        if(dms[n].num > 0) {
            for(int d = 0; d < dms[n].num; d++) {
                printf(" %d", dms[n].deep[d]);
            }
            printf("\n");
        } else {
            printf("\n");
        }
    }
    printf("\n");
}

void Vaddr_String::print_res_dms()
{
    if(this->analyzed() != 1) {
        printf("[%s-%s:%d] Something not analyze.\n", __FILE__, __func__, __LINE__);
        return;
    }
    
    printf("Variant Result Got:\n");
    for(int n = 0; n < vstr_part_num; n++) {
        printf("[%d] %s", n, vstr_part[n]);
        if(dms_file[n].num > 0) {
            for(int d = 0; d < dms_file[n].num; d++) {
                printf(" %d", dms_file[n].deep[d]);
            }
            printf("\n");
        } else {
            printf("\n");
        }

        printf("0x%02x 0x%04x\n", operation[n][0], operation[n][1]);
    }
    printf("\n");
}

void Vaddr_String::print_operation()
{
    if(this->analyzed() != 1) {
        printf("[%s-%s:%d] Something not analyze.\n", __FILE__, __func__, __LINE__);
        return;
    }

    printf("Variant Operation Got:\n");
    for(int n = 0; n < vstr_part_num; n++) {
        printf("[%d] %x %x\n", n, operation[n][0], operation[n][1]);
    }
    printf("\n");
}