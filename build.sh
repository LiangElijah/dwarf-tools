#!/usr/bin/bash

# mkdir build
# cd build
# cmake .. -DCMAKE_INSTALL_PREFIX=../output

gcc main.cpp dwarf_api.c vaddr_dwarf.cpp  vaddr_file.cpp  vaddr_string.cpp -o main -I../Wget/libdwarf/output/include/ -ldwarf -L../Wget/libdwarf/output/lib/ -lstdc++
