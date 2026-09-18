:: mkdir build
:: cd build
:: cmake .. -G "MinGW Makefiles" -DCMAKE_INSTALL_PREFIX=C:\WorkSpace\ByteStudio\libdwarf\output

gcc .\main.c .\dwarf_coff.c .\dwarf_elf.c .\dwarf_die.c .\dwarf_method.c .\dwarf_str.c .\dwarf_addr.c -o .\main -IC:\WorkSpace\ByteStudio\libdwarf\output\include -ldwarf -LC:\WorkSpace\ByteStudio\libdwarf\output\lib -DLIBDWARF_STATIC -lstdc++

::pause
