#include "vaddr_file.h"

Vaddr_File::Vaddr_File(const char *file_path)
{
    extract(file_path);
}

Vaddr_File::Vaddr_File()
{
    is_ready = 0;
}

Vaddr_File::~Vaddr_File()
{

}

int Vaddr_File::extract(const char *file_path)
{
    FILE *fp = NULL;
    long size = 0;
    st_filehdr_t filehdr = {0};
    int32_t i32StrOffset = 0;

    int res = 0;
    is_ready = 0;

    if(file_path == NULL) return -1;

    // 1、判断文件是否存在
    if(access(file_path, F_OK) != 0) {
        printf("[%s-%s:%d] access() %s.\n", __FILE__, __func__, __LINE__, strerror(errno));
        return -1;
    }
    memcpy(this->file_path, file_path, strlen(file_path) + 1);

    if(dwarf_is_elf(file_path) == 0) {
        file_type = FILE_ELF;
        is_ready = 1;
        return 0;
    }
    
    // 2、以二进制只读打开文件
    fp = fopen(file_path, "rb");
    if(fp == NULL) {
        printf("[%s-%s:%d] fopen() %s.\n", __FILE__, __func__, __LINE__, strerror(errno));
        return -1;
    }

    // 3、获取文件大小
    if(fseek(fp, 0, SEEK_END) != 0) {
        printf("[%s-%s:%d] fseek() %s.\n", __FILE__, __func__, __LINE__, strerror(errno));
        goto CLOSE;
    }
    size = ftell(fp);
    if(size == -1) {
        printf("[%s-%s:%d] ftell() %s.\n", __FILE__, __func__, __LINE__, strerror(errno));
        goto CLOSE;
    }
    if(fseek(fp, 0, SEEK_SET) != 0) {
        printf("[%s-%s:%d] fseek() %s.\n", __FILE__, __func__, __LINE__, strerror(errno));
        goto CLOSE;
    }
    file_size = size;

    // 4、读取文件头
    res = fread(&filehdr, 1, sizeof(st_filehdr_t), fp);
    if(res != sizeof(st_filehdr_t)) {
        printf("[%s-%s:%d] fread() %s.\n", __FILE__, __func__, __LINE__, strerror(errno));
        goto CLOSE;
    }

    if(filehdr.u16Version != 0xC2) {
        printf("[%s-%s:%d] Unsupported File Type (%x).\n", __FILE__, __func__, __LINE__, filehdr.u16Version);
        goto CLOSE;
    }

    if(filehdr.u16Flags & 0x0100) {
        byte_order = DW_END_little;
    } else {
        byte_order = DW_END_big;
    }

    // 5、获取字符串表偏移
    i32StrOffset = filehdr.i32SymbolOffset + filehdr.i32NumSymbol * sizeof(st_syment_t);
    if(filehdr.u16OptHdrSZ != 0) {
        // 5.1、跳过可选文件头
        int32_t i32SecOffset = sizeof(st_filehdr_t) + filehdr.u16OptHdrSZ;
        if(fseek(fp, i32SecOffset, SEEK_SET) != 0) {
            printf("[%s-%s:%d] fseek() %s.\n", __FILE__, __func__, __LINE__, strerror(errno));
            goto CLOSE;
        }
    }

    release();

    // 6、遍历字段表
    for(int i = 1; i <= filehdr.u16NumSec; i++) {
        char secname[128] = {0};
        st_sechdr_t sechdr = {0};

        res = fread(&sechdr, 1, sizeof(st_sechdr_t), fp);
        if(res != sizeof(st_sechdr_t)) {
            printf("[%s-%s:%d] fread() %s.\n", __FILE__, __func__, __LINE__, strerror(errno));
            goto FREE;
        }
        
        if(sechdr.u.s.u32Zero == 0) {
            long pos = ftell(fp);
            if(pos == -1) {
                printf("[%s-%s:%d] ftell() %s.\n", __FILE__, __func__, __LINE__, strerror(errno));
                goto FREE;
            }
            int offset = i32StrOffset + sechdr.u.s.u32Offset;
            if(fseek(fp, offset, SEEK_SET) != 0) {
                printf("[%s-%s:%d] fseek() %s.\n", __FILE__, __func__, __LINE__, strerror(errno));
                goto FREE;
            }
            for(int j = 0; j < 127; j++) {
                char c = 0;
                res = fread(&c, 1, 1, fp);
                if(res != 1) {
                    printf("[%s-%s:%d] fread() %s.\n", __FILE__, __func__, __LINE__, strerror(errno));
                    goto FREE;
                }
                if((c <= 32) || (c > 127)) break;
                secname[j] = c;
            }
            if(fseek(fp, pos, SEEK_SET) != 0) {
                printf("[%s-%s:%d] fseek() %s.\n", __FILE__, __func__, __LINE__, strerror(errno));
                goto FREE;
            }
        } else {
            memcpy(secname, sechdr.u.i8Name, 8);
        }

        if(strncmp(secname, ".debug_", strlen(".debug_")) == 0) {
            memcpy(sems[semnum].name, secname, strlen(secname)+1);
            sems[semnum].data = malloc(sechdr.i32Size);
            long pos = ftell(fp);
            if(pos == -1) {
                printf("[%s-%s:%d] ftell() %s.\n", __FILE__, __func__, __LINE__, strerror(errno));
                goto FREE;
            }
            int offset = sechdr.i32SecOffset;
            if(fseek(fp, offset, SEEK_SET) != 0) {
                printf("[%s-%s:%d] fseek() %s.\n", __FILE__, __func__, __LINE__, strerror(errno));
                goto FREE;
            }
            res = fread(sems[semnum].data, 1, sechdr.i32Size, fp);
            if(res != sechdr.i32Size) {
                printf("[%s-%s:%d] fread() %s.\n", __FILE__, __func__, __LINE__, strerror(errno));
                goto FREE;
            }
            if(fseek(fp, pos, SEEK_SET) != 0) {
                printf("[%s-%s:%d] fseek() %s.\n", __FILE__, __func__, __LINE__, strerror(errno));
                goto FREE;
            }
            sems[semnum].size = sechdr.i32Size;
            semnum++;
        }
    }

    if(semnum <= 0) {
        printf("[%s-%s:%d] Unsupported File Type (%x).\n", __FILE__, __func__, __LINE__, filehdr.u16Version);
        goto FREE;
    }

    file_type = FILE_COFF;
    is_ready = 1;
    fclose(fp);
    return 0;

FREE:
    release();
CLOSE:
    fclose(fp);

    return -1;
}

int Vaddr_File::ready()
{
    return is_ready;
}

void Vaddr_File::release()
{
    for(int i = 1; i <= semnum; i++) {
        free(sems[i].data);
    }
    semnum = 0;
}

void Vaddr_File::print()
{
    if(this->ready() != 1) {
        printf("[%s-%s:%d] Something not ready.\n", __FILE__, __func__, __LINE__);
        return;
    }
    
    if(file_type != FILE_COFF) return;
    
    printf("Section Got:\n");
    for(int n = 0; n < semnum; n++) {
        printf("[%d] %s\n", n, sems[n].name);
    }
    printf("\n");
}