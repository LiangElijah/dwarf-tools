#include "dwarf_elf.h"

int dwarf_elf_init(const char *path, 
    Dwarf_Obj *initSec_p,
    Dwarf_Debug *ret_dbg, 
    Dwarf_Error *error)
{
    FILE *fp = NULL;
    Fhdr fhdr = {0};
    uint64_t len = 0;

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

    readelf(fp, &fhdr);
    if(readelfsym(fp, "__TI_CINIT_Base", &fhdr) < 0)
    {
        printf("no found: __TI_CINIT_Base\r\n");
    }
    if(readelfsym(fp, "__TI_CINIT_Limit", &fhdr) < 0)
    {
        printf("no found: __TI_CINIT_Limit\r\n");
    }
    
    // initSec_p->data = readelfsection(fp, ".data", &len, &fhdr);
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

    // cinitSec.data = readelfsection(fp, ".cinit", &len, &fhdr);
    // if (cinitSec.data != NULL) {
    //     cinitSec.size = fhdr.size;
    //     cinitSec.addr = fhdr.addr;
    //     printf("%x %x %d\r\n", fhdr.offset, fhdr.addr, fhdr.size);
    //     for(int i = 0, n = 0; i < fhdr.size; i++) {
    //         printf("0x%02x ", cinitSec.data[i]); n++;
    //         if(n >= 8) {
    //             n = 0;
    //             printf("\r\n");
    //         }
    //         if(i == (fhdr.size - 1)) {
    //             printf("\r\n");
    //         }
    //     }
    // }
    // freeelf(&fhdr);

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