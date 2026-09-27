#include "dwarf_elf.h"

int dwarf_elf_init(const char *path, 
    Dwarf_Obj *initSec_p,
    Dwarf_Debug *ret_dbg, 
    Dwarf_Error *error)
{
    int res = dwarf_init_path(path, NULL, 0, DW_GROUPNUMBER_ANY, 
        NULL, NULL, ret_dbg, error);
    if(res != DW_DLV_OK) {
        goto RET;
    }

    FILE *fp = NULL;
    Fhdr fhdr = {0};
    uint64_t len = 0;

    fp = fopen(path, "rb");
    if(fp == NULL) {
        goto DEINIT;
    }

    initSec_p->data = readelfsection(fp, ".data", &len, &fhdr);
    if (initSec_p->data == NULL) {
        goto CLOSE;
    }
    initSec_p->size = fhdr.size;
    initSec_p->addr = fhdr.addr;

    fclose(fp);
    freeelf(&fhdr);

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