#!/usr/bin/bash

# mkdir build
# cd build
# cmake .. -DCMAKE_INSTALL_PREFIX=../output

gcc main.c elf.c beget.c leget.c dwarf_coff.c dwarf_elf.c dwarf_die.c dwarf_method.c dwarf_str.c dwarf_addr.c -o main -ID:/WorkSpace/Git/libdwarf/output/include -ldwarf-static -LD:/WorkSpace/Git/libdwarf/output/lib -DLIBDWARF_STATIC -lstdc++

: << 'EOF'

./main.exe test_coff_dwarf3.out var_func
./main.exe test_coff_dwarf3.out array_func
./main.exe test_coff_dwarf3.out array_func[1]
./main.exe test_coff_dwarf3.out var_func2
./main.exe test_coff_dwarf3.out array_func2
./main.exe test_coff_dwarf3.out array_func2[1]



./main.exe test_coff_dwarf3.out var
./main.exe test_coff_dwarf3.out var_ref
./main.exe test_coff_dwarf3.out array
./main.exe test_coff_dwarf3.out array[1]
./main.exe test_coff_dwarf3.out var_p
./main.exe test_coff_dwarf3.out array_p
./main.exe test_coff_dwarf3.out array_p[1]



./main.exe test_coff_dwarf3.out var_st1
./main.exe test_coff_dwarf3.out var_st1.a
./main.exe test_coff_dwarf3.out array_st1
./main.exe test_coff_dwarf3.out array_st1[1]
./main.exe test_coff_dwarf3.out array_st1[1].a
./main.exe test_coff_dwarf3.out var_st1_p
./main.exe test_coff_dwarf3.out array_st1_p
./main.exe test_coff_dwarf3.out array_st1_p[1]

./main.exe test_coff_dwarf3.out var_s2
./main.exe test_coff_dwarf3.out var_s2.b
./main.exe test_coff_dwarf3.out array_s2
./main.exe test_coff_dwarf3.out array_s2[1]
./main.exe test_coff_dwarf3.out array_s2[1].b
./main.exe test_coff_dwarf3.out var_s2_p
./main.exe test_coff_dwarf3.out array_s2_p
./main.exe test_coff_dwarf3.out array_s2_p[1]

./main.exe test_coff_dwarf3.out var_st3
./main.exe test_coff_dwarf3.out var_st3.c
./main.exe test_coff_dwarf3.out array_st3
./main.exe test_coff_dwarf3.out array_st3[1]
./main.exe test_coff_dwarf3.out array_st3[1].c
./main.exe test_coff_dwarf3.out var_st3_p
./main.exe test_coff_dwarf3.out array_st3_p
./main.exe test_coff_dwarf3.out array_st3_p[1]

./main.exe test_coff_dwarf3.out var_s3_2
./main.exe test_coff_dwarf3.out var_s3_2.d
./main.exe test_coff_dwarf3.out array_s3_2
./main.exe test_coff_dwarf3.out array_s3_2[1]
./main.exe test_coff_dwarf3.out array_s3_2[1].d
./main.exe test_coff_dwarf3.out var_s3_2_p
./main.exe test_coff_dwarf3.out array_s3_2_p
./main.exe test_coff_dwarf3.out array_s3_2_p[1]

./main.exe test_coff_dwarf3.out var_s4
./main.exe test_coff_dwarf3.out var_s4.b
./main.exe test_coff_dwarf3.out array_s4
./main.exe test_coff_dwarf3.out array_s4[1]
./main.exe test_coff_dwarf3.out array_s4[1].b
./main.exe test_coff_dwarf3.out var_s4_p
./main.exe test_coff_dwarf3.out array_s4_p
./main.exe test_coff_dwarf3.out array_s4_p[1]



./main.exe test_coff_dwarf3.out var_e1
./main.exe test_coff_dwarf3.out array_e1
./main.exe test_coff_dwarf3.out array_e1[1]
./main.exe test_coff_dwarf3.out var_e1_p
./main.exe test_coff_dwarf3.out array_e1_p
./main.exe test_coff_dwarf3.out array_e1_p[1]

./main.exe test_coff_dwarf3.out var_e2
./main.exe test_coff_dwarf3.out array_e2
./main.exe test_coff_dwarf3.out array_e2[1]
./main.exe test_coff_dwarf3.out var_e2_p
./main.exe test_coff_dwarf3.out array_e2_p
./main.exe test_coff_dwarf3.out array_e2_p[1]

./main.exe test_coff_dwarf3.out var_e3
./main.exe test_coff_dwarf3.out array_e3
./main.exe test_coff_dwarf3.out array_e3[1]
./main.exe test_coff_dwarf3.out var_e3_p
./main.exe test_coff_dwarf3.out array_e3_p
./main.exe test_coff_dwarf3.out array_e3_p[1]

./main.exe test_coff_dwarf3.out var_e3_2
./main.exe test_coff_dwarf3.out array_e3_2
./main.exe test_coff_dwarf3.out array_e3_2[1]
./main.exe test_coff_dwarf3.out var_e3_2_p
./main.exe test_coff_dwarf3.out array_e3_2_p
./main.exe test_coff_dwarf3.out array_e3_2_p[1]

./main.exe test_coff_dwarf3.out var_e4
./main.exe test_coff_dwarf3.out array_e4
./main.exe test_coff_dwarf3.out array_e4[1]
./main.exe test_coff_dwarf3.out var_e4_p
./main.exe test_coff_dwarf3.out array_e4_p
./main.exe test_coff_dwarf3.out array_e4_p[1]

EOF
