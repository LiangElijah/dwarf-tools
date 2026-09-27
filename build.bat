:: mkdir build
:: cd build
:: cmake .. -G "MinGW Makefiles" -DCMAKE_INSTALL_PREFIX=C:\WorkSpace\ByteStudio\libdwarf\output

gcc .\main.c .\dwarf_coff.c .\dwarf_elf.c .\dwarf_die.c .\dwarf_method.c .\dwarf_str.c .\dwarf_addr.c -o .\main -ID:\WorkSpace\Git\libdwarf\output\include -ldwarf-static -LD:\WorkSpace\Git\libdwarf\output\lib -DLIBDWARF_STATIC -lstdc++

::pause
