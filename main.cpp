#include "main.h"

static void vaddr_usage(const char* name) {
    fprintf(stderr, "Invalid argument, Supported Format:\n");
    fprintf(stderr, "%s -h \n", name);
    fprintf(stderr, "%s [-i]input_file [-v]variant\n", name);
}

int main(int argc, char *argv[])
{
    char copt = 0;
    int iopt = 0;
    char *cinput = NULL;
    char *cvariant = NULL;

    Dwarf_Debug dbg = NULL;
    Dwarf_Error error = NULL;
    Dwarf_Obj_Access_Data *dw_accessData = NULL;
    st_dieNode_t *entry = NULL;
    st_str_t str = {0};
    st_addr_t addr = {0};
    int res = 0;

    /*
     * 1、"c:"  必须有参数
     * 2、"c::" 参数可有可无，有参数不能有空格，必须写成 -cxxx 形式
     * 3、"c"   无参数
     * 4、"co"/"co:"/"co::" 可合并参数，如 -co
     * 5、不论书写顺序，- 开头参数选项在前，不以 - 开头参数自动在optind及后
     * 6、若 argc 减去 optind 大于 0，有额外参数
     * 7、出错或选项不在 optstr 内，返回?
     */
    while ((copt = getopt(argc, argv, "i::v::h")) != -1) {
        switch (copt) {
            //Common argv
            case 'i':
                cinput = optarg;
                break;
            case 'v':
                cvariant = optarg;
                break;
            //Other argv
            case 'h':
            case '?':
            default:
                vaddr_usage(argv[0]);
                return -1;
        }
    }

    iopt = optind;

    if(cinput == NULL)
    {
        if((argc - iopt) <= 0)
        {
            vaddr_usage(argv[0]);
            return -1;
        }
        else
        {
            cinput = argv[iopt];
        }

        iopt++;
    }

    if(cvariant == NULL)
    {
        if((argc - iopt) <= 0)
        {
            vaddr_usage(argv[0]);
            return -1;
        }
        else
        {
            cvariant = argv[iopt];
        }

        iopt++;
    }

    printf("Argument Got: -i %s -v %s\n\n", cinput, cvariant);

    /* Do Something Here */
    res = dwarf_elf_init(cinput, &dbg, &error);
    if(res != DW_DLV_OK)
    {
        res = dwarf_coff_init(cinput, &dw_accessData, &dbg, &error);
        if(res != DW_DLV_OK)
        {
            printf("file format err\r\n");
            return 0;
        }
    }

    res = dwarf_die_init(dbg, &entry, &error);
    // if((res == DW_DLV_OK) && (entry != NULL))
    //     dwarf_print_die(dbg, entry, &error);
    // else printf("entry err\r\n");

    res = dwarf_str_init(cvariant, &str);
    // if(res == 0)
    //     dwarf_print_str(&str);
    // else printf("str err\r\n");

    res = dwarf_addr_cal(entry, &str, &addr);
    if(res == 0)
        dwarf_print_addr(&addr);
    else printf("addr err\r\n");

    return 0;
}
