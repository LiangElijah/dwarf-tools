:: mkdir build
:: cd build
:: cmake .. -G "MinGW Makefiles" -DCMAKE_INSTALL_PREFIX=C:\WorkSpace\ByteStudio\libdwarf\output

gcc .\main.cpp .\dwarf_api.c .\vaddr_dwarf.cpp  .\vaddr_file.cpp  .\vaddr_string.cpp -o .\main -IC:\WorkSpace\ByteStudio\libdwarf\output\include -ldwarf -LC:\WorkSpace\ByteStudio\libdwarf\output\lib -DLIBDWARF_STATIC -lstdc++
