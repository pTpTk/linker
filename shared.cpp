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
    0x00, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
}

extern std::vector<char> texts;
extern std::unordered_multimap<std::string, uint64_t> rels;
extern std::unordered_set<std::string> refs;

std::vector<char> dynsym;
std::string dynstr;

std::vector<uint8_t> plt;
std::vector<uint64_t> got;

std::vector<char> rela_plt;

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

    for(int i = 0x18; i < dynsym.size(); i+=0x18) {
        uint32_t st_name  = (uint32_t&)dynsym[i];
        uint8_t  st_info  = (uint8_t&) dynsym[i+0x4];
        uint16_t st_shndx = (uint16_t&)dynsym[i+0x6];
        uint64_t st_value = (uint32_t&)dynsym[i+0x8];

        std::string name(dynstr.data() + st_name);

        // Only support self-contained libs
        assert(st_shndx);
        symbols.emplace_back(name, st_value);
    }

    D("%s tags:\n", f.file_name.c_str());
    for(auto& s : symbols) {
        D("%s: 0x%lX\n", s.name.c_str(), s.val);
    }
}

template<typename T>
inline void align16(T& offset) {
    if(offset & 0b1111) {
        offset >>= 4;
        ++offset;
        offset <<= 4;
    }
}

std::vector<uint8_t> PLT_0
{
    0xFF, 0x35, 0x00, 0x00, 0x00, 0x00, /* push got[0x8] */
    0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, /* jmp got[0x10] */
    0x0f, 0x1f, 0x40, 0x00              /* nop */
};
int& got_08_offset = (int&)PLT_0[2];
int& got_10_offset = (int&)PLT_0[8];

void plt_init(uint64_t got_addr) {
    uint plt_addr = texts.size() + 0x1000;
    align16(plt_addr);

    got_08_offset = got_addr + 0x08 - plt_addr - 0x6;
    got_10_offset = got_addr + 0x10 - plt_addr - 0xC;
}

std::vector<uint8_t> plt_entry
{
    0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, /* jmp got[tag] */
    0x68, 0x00, 0x00, 0x00, 0x00,       /* push sym_index*/
    0xE9, 0xE0, 0xFF, 0xFF, 0xFF        /* jmp plt_0 */
};
int& got_tag_offset = (int&)plt_entry[2];
int& sym_index      = (int&)plt_entry[7];
int& plt_0_offset   = (int&)plt_entry[12];

void plt_entry_init(uint64_t plt_addr, uint64_t got_addr) {
    uint64_t plt_1_addr = plt_addr + 0x10;
    uint64_t got_1_addr = got_addr + 0x18;
    got_tag_offset = got_1_addr - plt_1_addr - 0x6;
}

void plt_entry_advance() {
    got_tag_offset -= 0x8;
    sym_index += 1;
    plt_0_offset -= 0x10;
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
    }

    uint64_t plt_offset = texts.size() + 0x1000;
    align16(plt_offset);

    uint64_t plt_entry_offset = plt_offset + 0x10 - 0x1000;

    uint got_entry = plt_offset + 0x16;

    int sym_index = 1;

    uint got_entry_addr = 0x2000 + (12 << 4) + 0x18;
    plt_entry_init(plt_offset, 0x2000 + (12 << 4));

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
        merge(plt, plt_entry);
        plt_entry_advance();

        // got
        got.emplace_back(got_entry);
        got_entry += 0x10;

        // apply plt relocations
        auto range = rels.equal_range(s);
        for(auto it = range.first; it != range.second; ++it) {
            int& imm = (int&)texts[it->second];
            int S = plt_entry_offset;
            int P = it->second + sizeof(int);
            imm = S - P;

            D("relocation @ 0x%lx, target addr 0x%lx\n", 
                it->second, plt_entry_offset);
        }
        plt_entry_offset += 0x10;

        // rela.plt
        std::vector<char> rela_plt_entry(0x18,0);
        uint64_t& r_offset = (uint64_t&)rela_plt_entry[0x00];
        uint32_t& r_type   = (uint32_t&)rela_plt_entry[0x08];
        uint32_t& r_sym    = (uint32_t&)rela_plt_entry[0x0C];
        r_offset = got_entry_addr;
        r_type   = 0x07;
        r_sym    = sym_index;
        merge(rela_plt, rela_plt_entry);
        got_entry_addr += 0x08;
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

    dynsym = std::vector<char>(0x18,0);
    dynstr += '\0';
    plt_init(0x2000 + (12 << 4));
    plt = PLT_0;
    got = {0x2000, 0x00, 0x00};
    for(auto& f : libs) {
        WriteDyns(f);
    }

}