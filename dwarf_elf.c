#include "dwarf_elf.h"

int dwarf_elf_init(const char *path, 
    Dwarf_Obj *initSec_p,
    Dwarf_Debug *ret_dbg, 
    Dwarf_Error *error)
{
    FILE *fp = NULL;
    Fhdr fhdr = {0};

    Dwarf_Obj cinitSec = {0};
    
    int res = dwarf_init_path(path, NULL, 0, DW_GROUPNUMBER_ANY, 
        NULL, NULL, ret_dbg, error);
    if(res != DW_DLV_OK) {
        goto RET;
    }

    fp = fopen(path, "rb");
    if(fp == NULL) {
        goto DEINIT;
    }

    uint64_t __TI_CINIT_Base;
    uint64_t __TI_CINIT_Limit;
    uint64_t __TI_Handler_Table_Base;
    uint64_t __TI_Handler_Table_Limit;
    uint64_t __TI_decompress_none;
    uint64_t __TI_decompress_lzss;
    uint64_t __TI_zero_init_nomemset;

    readelf(fp, &fhdr);

    if(readelfsym(fp, "__TI_CINIT_Base", &fhdr) < 0)
    {
        printf("no found: __TI_CINIT_Base\r\n");
    }
    printf("__TI_CINIT_Base:%x\r\n", fhdr.value);
    __TI_CINIT_Base = fhdr.value;

    if(readelfsym(fp, "__TI_CINIT_Limit", &fhdr) < 0)
    {
        printf("no found: __TI_CINIT_Limit\r\n");
    }
    printf("__TI_CINIT_Limit:%x\r\n", fhdr.value);
    __TI_CINIT_Limit = fhdr.value;

    if(readelfsym(fp, "__TI_Handler_Table_Base", &fhdr) < 0)
    {
        printf("no found: __TI_Handler_Table_Base\r\n");
    }
    printf("__TI_Handler_Table_Base:%x\r\n", fhdr.value);
    __TI_Handler_Table_Base = fhdr.value;

    if(readelfsym(fp, "__TI_Handler_Table_Limit", &fhdr) < 0)
    {
        printf("no found: __TI_Handler_Table_Limit\r\n");
    }
    printf("__TI_Handler_Table_Limit:%x\r\n", fhdr.value);
    __TI_Handler_Table_Limit = fhdr.value;

    if(readelfsym(fp, "__TI_decompress_none", &fhdr) < 0)
    {
        printf("no found: __TI_decompress_none\r\n");
    }
    printf("__TI_decompress_none:%x\r\n", fhdr.value);
    __TI_decompress_none = fhdr.value;

    if(readelfsym(fp, "__TI_decompress_lzss", &fhdr) < 0)
    {
        printf("no found: __TI_decompress_lzss\r\n");
    }
    printf("__TI_decompress_lzss:%x\r\n", fhdr.value);
    __TI_decompress_lzss = fhdr.value;

    if(readelfsym(fp, "__TI_zero_init_nomemset", &fhdr) < 0)
    {
        printf("no found: __TI_zero_init_nomemset\r\n");
    }
    printf("__TI_zero_init_nomemset:%x\r\n", fhdr.value);
    __TI_zero_init_nomemset = fhdr.value;
    
    // initSec_p->data = readelfsect(fp, ".data", &len, &fhdr);
    // if (initSec_p->data == NULL) {
    //     goto CLOSE;
    // }
    // initSec_p->size = fhdr.size;
    // initSec_p->addr = fhdr.addr;
    // printf("%x %x %d\r\n", fhdr.offset, fhdr.addr, fhdr.size);
    // for(int i = 0, n = 0; i < fhdr.size; i++) {
    //     printf("0x%02x ", initSec_p->data[i]); n++;
    //     if(n >= 8) {
    //         n = 0;
    //         printf("\r\n");
    //     }
    //     if(i == (fhdr.size - 1)) {
    //         printf("\r\n");
    //     }
    // }
    // freeelf(&fhdr);

    cinitSec.data = readelfsect(fp, ".cinit", &fhdr);
    if (cinitSec.data != NULL) {
        cinitSec.size = fhdr.size;
        cinitSec.addr = fhdr.addr;
        printf("%x %x %d\r\n", fhdr.offset, fhdr.addr, fhdr.size);
        for(int i = 0, n = 0; i < fhdr.size; i++) {
            printf("0x%02x ", cinitSec.data[i]); n++;
            if(n >= 8) {
                n = 0;
                printf("\r\n");
            }
            if(i == (fhdr.size - 1)) {
                printf("\r\n");
            }
        }
    }

    if((__TI_CINIT_Base > __TI_CINIT_Limit) || (cinitSec.addr > __TI_CINIT_Base) || 
        ((__TI_CINIT_Limit - cinitSec.addr) > (cinitSec.size/2))) {
        return -1;
    }

    if((__TI_Handler_Table_Base > __TI_Handler_Table_Limit) || (cinitSec.addr > __TI_Handler_Table_Base) || 
        ((__TI_Handler_Table_Limit - cinitSec.addr) > (cinitSec.size/2))) {
        return -1;
    }
    
    typedef void (*handler_fn_t)(char const *in, char *out);
    handler_fn_t handler_array[4];

    void dwarf_elf_decompress_none(char const *in, char *out);
    void dwarf_elf_decompress_lzss(char const *in, char *out);
    void dwarf_elf_zero_init(char const *in, char *out);

    uint32_t *handler_table = (uint32_t *)((uint16_t *)cinitSec.data + (__TI_Handler_Table_Base - cinitSec.addr));
    for(int i = 0; i < ((__TI_Handler_Table_Limit - __TI_Handler_Table_Base)/2); i++) {
        if(handler_table[i] == __TI_decompress_none) {
            handler_array[i] = dwarf_elf_decompress_none;
        } else if(handler_table[i] == __TI_decompress_lzss) {
            handler_array[i] = dwarf_elf_decompress_none;
        } else if(handler_table[i] == __TI_zero_init_nomemset) {
            handler_array[i] = dwarf_elf_zero_init;
        } else {
            printf("unsupport handler\r\n");
            return -1;
        }
    }

    uint32_t *cinit_table = (uint32_t *)((uint16_t *)cinitSec.data + (__TI_CINIT_Base - cinitSec.addr));
    for(int i = 0; i < ((__TI_CINIT_Limit - __TI_CINIT_Base)/4); i++)
    {
        unsigned long load_addr = *cinit_table++;
        unsigned long run_addr  = *cinit_table++;
        printf("%x %x\r\n", load_addr, run_addr);
        if((load_addr < cinitSec.addr) || ((load_addr - cinitSec.addr) > (cinitSec.size/2))) {
            return -1;
        }
        uint16_t *handler_idx_p = (uint16_t *)cinitSec.data + (load_addr - cinitSec.addr);
        uint16_t handler_idx = *handler_idx_p;
        printf("index:%d\r\n", handler_idx);
        handler_fn_t handler = handler_array[handler_idx];
        handler((char const *)load_addr, (char *)run_addr);
    }

    freeelf(&fhdr);
    fclose(fp);
    return DW_DLV_OK;

CLOSE:
    fclose(fp);
DEINIT:
    dwarf_finish(*ret_dbg); 
RET:
    return DW_DLV_ERROR;
}

void dwarf_elf_deinit(Dwarf_Debug dw_dbg)
{
    dwarf_finish(dw_dbg); 
}

// #include <stdint.h>

// #define LZSS_EOD 0x0FFF // 假设 12 位高位全 1 为结束符

// void lzss_decompress_c2000(const uint16_t *src, uint16_t *dst) {
//     uint16_t src_ptr = 0;
//     uint16_t dst_ptr = 0;
    
//     while (1) {
//         // 1. 读取 16 位控制标志 F
//         uint16_t F = src[src_ptr++];
        
//         // 2. 依次处理 F 中的 16 个 bit
//         for (int bit = 0; bit < 16; bit++) {
//             uint16_t B = F & 0x0001;
//             F >>= 1; // 准备下一个 bit
            
//             if (B & 0x1) {
//                 // a. 未压缩：读取 16 位并直接写入输出缓冲区
//                 dst[dst_ptr++] = src[src_ptr++];
//             } 
//             else {
//                 // b. 压缩数据：读取 16 位 T
//                 uint16_t T = src[src_ptr++];
                
//                 uint16_t L = (T & 0x000F) + 2; // 低 4 位为长度
//                 uint16_t O = T >> 4;           // 高 12 位为偏移量
                
//                 // i. 长度扩展检查
//                 if (L == 17) {
//                     uint16_t L_prime = src[src_ptr++];
//                     L += L_prime;
//                 }
                
//                 // ii. 检查是否到达数据末尾
//                 if (O == LZSS_EOD) {
//                     return; // 结束解压
//                 }
                
//                 // iii. 滑动窗口复制 (以 16 位 Word 为单位)
//                 uint16_t P = dst_ptr - O - 1;
//                 for (uint16_t i = 0; i < L; i++) {
//                     dst[dst_ptr++] = dst[P++];
//                 }
//             }
//         }
//     }
// }

void dwarf_elf_decompress_none(char const *in, char *out)
{
    printf("%s\r\n", __func__);
}
void dwarf_elf_decompress_lzss(char const *in, char *out)
{
    printf("%s\r\n", __func__);
}
void dwarf_elf_zero_init(char const *in, char *out)
{
    printf("%s\r\n", __func__);
}
