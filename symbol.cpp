#include <vector>
#include <string>
#include <iostream>
#include <cassert>
#include <unordered_map>

#include "debug.h"
#include "object.h"

std::vector<char> texts;

namespace {

std::unordered_map<std::string, int> global_symbols;

inline bool IsGlobal(uint8_t st_info) {
    char st_bind = st_info >> 4;
    return st_bind == 1 || st_bind == 2;
}
inline bool IsDefined(uint16_t st_shndx) { return st_shndx; }

void GetSymbols(ObjectFile& f) {
    std::vector<char>& symtab = f.symtab;
    std::vector<char>& strtab = f.strtab;
    std::vector<Symbol>& symbols = f.symbols;

    for(int i = 0x10; i < symtab.size(); i+=0x10) {
        uint32_t st_name  = (uint32_t&)symtab[i];
        uint32_t st_value = (uint32_t&)symtab[i+0x4];
        uint8_t  st_info  = (uint8_t&) symtab[i+0xC];
        uint16_t st_shndx = (uint16_t&)symtab[i+0xE];

        std::string name(strtab.data() + st_name);

        if(IsDefined(st_shndx)) {
            symbols.emplace_back(name, st_value);
            if(IsGlobal(st_info))
                global_symbols[name] = st_value + f.text_offset;
        }
        else {
            symbols.emplace_back(name);
        }
    }

    D("tags:\n");
    for(auto& s : symbols) {
        D("%s: 0x%X\n", s.name.c_str(), s.val);
    }
}

void Relocate(ObjectFile& f) {
    D("filename: %s\n", f.file_name.c_str());

    std::vector<char>& text = f.text;
    std::vector<char>& rel_text = f.rel_text;
    std::vector<Symbol>& symbols = f.symbols;

    for(int i = 0; i < rel_text.size(); i+=8) {
        uint r_offset = (uint&) rel_text[i];
        char r_type = rel_text[i+4];
        char r_sym  = rel_text[i+5];

        assert(r_type == 2);
        D("r_offset = %x, r_sym = %x\n", r_offset, r_sym);

        int& imm = (int&)text[r_offset];
        int S;
        int P = r_offset + sizeof(int);

        Symbol s = symbols[r_sym-1];
        D("symbol: %s\n", s.name.c_str());

        if(s.defined) {
            S = s.val;
        }
        else {
            auto entry = global_symbols.find(s.name);
            assert(entry != global_symbols.end());

            S = global_symbols[s.name];
        }

        imm = S - P;

        D("S = 0x%X, P = 0x%X\n", S, P);
    }
}

void SetTextOffsets(std::vector<ObjectFile>& files) {
    int offset = 0;
    for(auto& f : files) {
        f.text_offset = offset;
        offset += f.text.size();
    }
}

void CollectTexts(std::vector<ObjectFile>& files) {
    for(auto& f : files) {
        texts.insert(texts.end(), f.text.begin(), f.text.end());
    }
}

} //namespace

void ResolveSymbols(std::vector<ObjectFile>& files) {
    SetTextOffsets(files);

    for(auto& f : files) {
        GetSymbols(f);
    }
    for(auto& f : files) {
        Relocate(f);
    }

    for(auto& f : files) {
        int i = 0;
        for(auto& c : f.text) {
            D("%02x ", (unsigned char)c);
            if(++i % 16 == 0) D("\n");
        }
        D("\n");
    }

    CollectTexts(files);
}