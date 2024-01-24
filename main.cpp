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
    // 1、分解变量字符串
    Vaddr_String str(cvariant);

    str.print_dms();

    // 2、提取文件的 dwarf 信息
    Vaddr_File file(cinput);

    file.print();

    // 3、创建 dwarf 解析器
    Vaddr_Dwarf dwarf(&file);

    dwarf.print();

    // 4、计算变量信息
    dwarf.analyze(&str);

    str.print_res_dms();

    // 4、计算变量地址
    str.calculate();

    str.print_cal();

    return 0;
}
