#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>

#include "debug.h"
#include "object.h"
#include "dynamic.h"

// Only support STT_FUNC type
#define EMPTY_SYMTAB_EMTRY \
{ \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00 \
}

#define PLT_0 \
{ \
    0xFF, 0xB3, 0x04, 0x00, 0x00, 0x00, 0xFF, 0xA3, \
    0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
}

#define PLT_ENTRY \
{ \
    0xFF, 0xA3, 0x0C, 0x00, 0x00, 0x00, 0x68, 0x00, \
    0x00, 0x00, 0x00, 0xE9, 0xE0, 0xFF, 0xFF, 0xFF \
}

extern std::vector<char> texts;
extern std::unordered_multimap<std::string, uint> rels;
extern std::unordered_set<std::string> refs;

std::vector<char> dynsym;
std::string dynstr;

std::vector<uint8_t> plt;
std::vector<uint> got;

std::vector<char> rel_dyn;
std::vector<char> rel_plt;

Dynamic dynamic;

namespace {

template<typename T>
inline void merge(std::vector<T>& v1, 
                  std::vector<T>& v2) {
    v1.insert(v1.end(), v2.begin(), v2.end());
}

void GetSymbols(LibFile& f) {
    auto& dynsym = f.dynsym;
    auto& dynstr = f.dynstr;
    auto& symbols = f.symbols;

    for(int i = 0x10; i < dynsym.size(); i+=0x10) {
        uint32_t st_name  = (uint32_t&)dynsym[i];
        uint32_t st_value = (uint32_t&)dynsym[i+0x4];
        uint8_t  st_info  = (uint8_t&) dynsym[i+0xC];
        uint16_t st_shndx = (uint16_t&)dynsym[i+0xE];

        std::string name(dynstr.data() + st_name);

        // Only support self-contained libs
        assert(st_shndx);
        symbols.emplace_back(name, st_value);
    }

    D("%s tags:\n", f.file_name.c_str());
    for(auto& s : symbols) {
        D("%s: 0x%X\n", s.name.c_str(), s.val);
    }
}

inline void align16(uint& offset) {
    if(offset & 0b1111) {
        offset >>= 4;
        ++offset;
        offset <<= 4;
    }
}

void WriteDyns(LibFile& f) {
    auto& fsymbols = f.symbols;
    std::vector<std::string> referred_symbols;

    for(auto r : refs) {
        for(auto& s : fsymbols) {
            if(s.name == r) {
                referred_symbols.emplace_back(r);
                break;
            }
        }
        printf("\n");
    }

    uint got_offset = 0x0c;
    uint push_val = 0x00;
    uint plt0_offset = -0x20;

    uint text_offset = texts.size() + 0x1000;
    align16(text_offset);
    text_offset += 0x16;

    int sym_index = 1;

    uint got_entry_addr = 0x2000 + (17 << 3) + 0x0c;

    for(auto& s : referred_symbols) {
        // dynsym
        std::vector<char> dynsym_entry = EMPTY_SYMTAB_EMTRY;
        uint& pos  = (uint&)dynsym_entry[0];
        pos = dynstr.size();
        merge(dynsym, dynsym_entry);

        // dynstr
        dynstr += s;
        dynstr += '\0';
        refs.erase(s);

        // plt
        std::vector<uint8_t> plt_entry = PLT_ENTRY;
        uint& entry_got_offset  = (uint&)plt_entry[2];
        uint& entry_push_val    = (uint&)plt_entry[7];
        uint& entry_plt0_offset = (uint&)plt_entry[12];
        entry_got_offset  = got_offset;
        entry_push_val    = push_val;
        entry_plt0_offset = plt0_offset;
        got_offset  += 0x04;
        push_val    += 0x08;
        plt0_offset -= 0x10;
        merge(plt, plt_entry);

        // got
        got.emplace_back(text_offset);
        text_offset += 0x10;

        // rel.dyn
        auto range = rels.equal_range(s);
        for(auto it = range.first; it != range.second; ++it) {
            std::vector<char> rel_dyn_entry(8,0);
            uint32_t& r_offset = (uint&)rel_dyn_entry[0];
            char&  r_type      =        rel_dyn_entry[4];
            char&  r_sym       =        rel_dyn_entry[5];
            r_offset = it->second + 0x1000;
            r_type   = 0x02;
            r_sym    = sym_index;
            merge(rel_dyn, rel_dyn_entry);
        }

        // rel.plt
        std::vector<char> rel_plt_entry(8,0);
        uint32_t& r_offset = (uint&)rel_plt_entry[0];
        char&  r_type      =        rel_plt_entry[4];
        char&  r_sym       =        rel_plt_entry[5];
        r_offset = got_entry_addr;
        r_type   = 0x07;
        r_sym    = sym_index;
        merge(rel_plt, rel_plt_entry);
        got_entry_addr += 0x04;
        sym_index += 1;
    }

    dynamic.needed = dynstr.size();

    dynstr += f.file_name;
    dynstr += '\0';

}

}

void ProcessSharedLibs(std::vector<LibFile>& libs) {
    for(auto& f : libs) {
        GetSymbols(f);
    }

    dynsym = std::vector<char>(0x10,0);
    dynstr += '\0';
    plt = PLT_0;
    got = {0x2000, 0x00, 0x00};
    for(auto& f : libs) {
        WriteDyns(f);
    }

}