#ifndef __VADDR_STRING_H__
#define __VADDR_STRING_H__

#include <cstdio>  // printf ...
#include <cstdint> // uint32_t ...
#include <cstdlib> // malloc
#include <cerrno>  // errno
#include <cstring> // strerror

extern "C" {
   #include <unistd.h> // getopt、access

   #include "dwarf_api.h"
}

typedef struct st_dms {
    uint32_t num;
    uint32_t size;
    uint32_t deep[4];
} st_dms_t;

class Vaddr_Dwarf;
class Vaddr_String
{
    friend class Vaddr_Dwarf;
private:
    char is_ready = 0;
    char is_analyzed = 0;
    char is_caled = 0;

    char vstr[512] = {0};
    char vstr_part[16][32] = {0};
    char vstr_part_num = 0;
    char vtype[16] = {0};
    
    st_dms_t dms[16] = {0};
    st_dms_t dms_file[16] = {0};
    int32_t operation[16][4] = {0};
    int32_t bitsize = 0;
    int32_t bitoffset = 0;
    uint32_t objaddr = 0;
    
public:
    Vaddr_String(const char *vstr);
    Vaddr_String();
    ~Vaddr_String();

    int parse(const char *vstr);
    int ready();
    int analyzed();
    int caled();
    int calculate();
    void print_cal();
    void print_dms();
    void print_res_dms();
    void print_operation();
};

#endif /* __VADDR_STRING_H__ */
