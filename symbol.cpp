#include <vector>
#include <string>
#include <iostream>
#include <cassert>

class Section
{
  public:
    std::string name;
    std::vector<char> bin;

    void alloc(uint size) { bin.resize(size); }
};

struct Symbol
{
    std::string name;
    int val;

    Symbol(std::string n, int v)
    : name(n), val(v) {}
};

extern std::vector<Section> sections;
std::vector<Symbol> symbols;

void GetSymbols() {
    Section symtab;
    Section strtab;

    for(auto& s : sections) {
        if(s.name == ".symtab") {
            symtab = s;
            continue;
        }
        if(s.name == ".strtab") {
            strtab = s;
            continue;
        }
    }

    for(int i = 0x10; i < symtab.bin.size(); i+=0x10) {
        int st_name  = (int&)symtab.bin[i];
        int st_value = (int&)symtab.bin[i+4];

        std::string name(strtab.bin.data() + st_name);

        symbols.emplace_back(name, st_value);
    }

    for(auto& s : symbols) {
        std::cout << s.name << std::endl;
    }
}

extern Section text;

void Relocate() {
    std::vector<char> rel_text;
    for(auto& s : sections) {
        if(s.name == ".rel.text") {
            rel_text = s.bin;
            break;
        }
    }

    for(int i = 0; i < rel_text.size(); i+=8) {
        uint r_offset = (uint&) rel_text[i];
        char r_type = rel_text[i+4];
        char r_sym  = rel_text[i+5];

        assert(r_type == 2);
        printf("r_offset = %x, r_sym = %x\n", r_offset, r_sym);

        int& imm = (int&)text.bin[r_offset];
        int S = symbols[r_sym-1].val;
        int P = r_offset + sizeof(int);
        imm = S - P;

        printf("S = %x, P = %x\n", S, P);
    }
}

void ResolveSymbols() {
    GetSymbols();
    Relocate();
}