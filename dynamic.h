#pragma once

#include <sys/types.h>
#include <cstdint>
#include <vector>

class Dynamic
{
    uint64_t NEEDED    = 0x00000001;
    uint64_t GNU_HASH  = 0x6ffffef5;
    uint64_t STRTAB    = 0x00000005;
    uint64_t SYMTAB    = 0x00000006;
    uint64_t STRSZ     = 0x0000000a;
    uint64_t SYMENT    = 0x0000000b;
    uint64_t PLTGOT    = 0x00000003;
    uint64_t PLTRELSZ  = 0x00000002;
    uint64_t PLTREL    = 0x00000014;
    uint64_t JMPREL    = 0x00000017;
    uint64_t FLAGS_1   = 0x6ffffffb;
  public:
    uint64_t needed;
    uint64_t gnu_hash;
    uint64_t strtab;
    uint64_t symtab;
    uint64_t strsz;
    uint64_t syment = 0x10;
    uint64_t pltgot;
    uint64_t pltrelsz;
    uint64_t pltrel = 0x07;
    uint64_t jmprel;
    uint64_t flags_1 = 0x08000000;

    std::vector<char> output;

    inline void add(uint64_t name, uint64_t val) {
        std::vector<char> v(16);
        uint64_t& v_name = (uint64_t&)v[0];
        uint64_t& v_val  = (uint64_t&)v[8];
        v_name = name;
        v_val  = val;
        
        output.insert(output.end(), v.begin(), v.end());
    }

    void generate() {
        output.clear();
        add(NEEDED   , needed);
        add(GNU_HASH , gnu_hash);
        add(STRTAB   , strtab);
        add(SYMTAB   , symtab);
        add(STRSZ    , strsz);
        add(SYMENT   , syment);
        add(PLTGOT   , pltgot);
        add(PLTRELSZ , pltrelsz);
        add(PLTREL   , pltrel);
        add(JMPREL   , jmprel);
        add(FLAGS_1  , flags_1);
        add(0, 0);
    }
};