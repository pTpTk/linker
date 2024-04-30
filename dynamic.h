#pragma once

#include <sys/types.h>
#include <cstdint>
#include <vector>

class Dynamic
{
    uint NEEDED    = 0x00000001;
    uint GNU_HASH  = 0x6ffffef5;
    uint STRTAB    = 0x00000005;
    uint SYMTAB    = 0x00000006;
    uint STRSZ     = 0x0000000a;
    uint SYMENT    = 0x0000000b;
    uint PLTGOT    = 0x00000003;
    uint PLTRELSZ  = 0x00000002;
    uint PLTREL    = 0x00000014;
    uint JMPREL    = 0x00000017;
    uint REL       = 0x00000011;
    uint RELSZ     = 0x00000012;
    uint RELENT    = 0x00000013;
    uint TEXTREL   = 0x00000016;
    uint FLAGS     = 0x0000001e;
    uint FLAGS_1   = 0x6ffffffb;
  public:
    uint needed;
    uint gnu_hash;
    uint strtab;
    uint symtab;
    uint strsz;
    uint syment = 0x10;
    uint pltgot;
    uint pltrelsz;
    uint pltrel = 0x11;
    uint jmprel;
    uint rel;
    uint relsz;
    uint relent = 0x08;
    uint textrel = 0x00;
    uint flags = 0x0c;
    uint flags_1 = 0x08000001;

    std::vector<char> output;

    inline void add(uint name, uint val) {
        std::vector<char> v(8);
        uint& v_name = (uint&)v[0];
        uint& v_val  = (uint&)v[4];
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
        add(REL      , rel);
        add(RELSZ    , relsz);
        add(RELENT   , relent);
        add(TEXTREL  , textrel);
        add(FLAGS    , flags);
        add(FLAGS_1  , flags_1);
        add(0, 0);
    }
};