#include <vector>
#include <string>
#include <unordered_set>

#include "debug.h"
#include "object.h"

extern std::vector<char> texts;
extern std::vector<Symbol> rels;
extern std::unordered_set<std::string> refs;

std::vector<char> dynsym;
std::string dynstr;

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

    for(auto& s : referred_symbols) {
        dynstr += s;
        dynstr += '\0';
        refs.erase(s);
    }

    dynstr += f.file_name;
    dynstr += '\0';

}

void ProcessSharedLibs(std::vector<LibFile>& libs) {
    for(auto& f : libs) {
        GetSymbols(f);
    }

    dynstr += '\0';
    for(auto& f : libs) {
        WriteDyns(f);
    }

}