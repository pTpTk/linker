#include <vector>
#include <string>
#include <iostream>
#include <cassert>

#include "debug.h"

extern std::vector<char> text;
extern std::vector<char> symtab;
extern std::vector<char> strtab;
extern std::vector<char> rel_text;

namespace {

struct Symbol
{
    std::string name;
    int val;

    Symbol(std::string n, int v)
    : name(n), val(v) {}
};

std::vector<Symbol> symbols;

void GetSymbols() {

    for(int i = 0x10; i < symtab.size(); i+=0x10) {
        int st_name  = (int&)symtab[i];
        int st_value = (int&)symtab[i+4];

        std::string name(strtab.data() + st_name);

        symbols.emplace_back(name, st_value);
    }

    for(auto& s : symbols) {
        std::cout << s.name << std::endl;
    }
}

void Relocate() {
    for(int i = 0; i < rel_text.size(); i+=8) {
        uint r_offset = (uint&) rel_text[i];
        char r_type = rel_text[i+4];
        char r_sym  = rel_text[i+5];

        assert(r_type == 2);
        printf("r_offset = %x, r_sym = %x\n", r_offset, r_sym);

        int& imm = (int&)text[r_offset];
        int S = symbols[r_sym-1].val;
        int P = r_offset + sizeof(int);
        imm = S - P;

        printf("S = %x, P = %x\n", S, P);
    }
}

} //namespace

void ResolveSymbols() {
    GetSymbols();
    Relocate();
}