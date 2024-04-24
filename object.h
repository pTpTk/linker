#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <cassert>

#include "debug.h"

struct Symbol
{
    std::string name;
    int val;
    bool defined;
    uint offset;

    Symbol(std::string n, int v)
    : name(n), val(v), defined(true), offset(0) {}

    Symbol(std::string n)
    : name(n), val(-1), defined(false), offset(0) {}
};

class ObjectFile
{

struct Section_Header
{
    std::vector<char> bin = std::vector<char>(0x28,0);
    int& sh_name    = (int&)bin[0x00];
    int& sh_type    = (int&)bin[0x04];
    int& sh_flags   = (int&)bin[0x08];
    int& sh_addr    = (int&)bin[0x0C];
    int& sh_offset  = (int&)bin[0x10];
    int& sh_size    = (int&)bin[0x14];
    int& sh_link    = (int&)bin[0x18];
    int& sh_info    = (int&)bin[0x1C];
    int& sh_align   = (int&)bin[0x20];
    int& sh_entsize = (int&)bin[0x24];
};

class Section
{
  public:
    std::string name;
    std::vector<char> bin;

    void alloc(uint size) { bin.resize(size); }
};

  public:
    std::ifstream ifs;
    std::string file_name;

    std::vector<char> file_header = std::vector<char>(0x34);
    uint32_t& e_shoff    = (uint32_t&)file_header[0x20];
    uint16_t& e_shnum    = (uint16_t&)file_header[0x30];
    uint16_t& e_shstrndx = (uint16_t&)file_header[0x32];

    std::vector<Section_Header> section_headers;
    std::vector<char> shstrtab;
    std::vector<Section> sections;

    int text_offset;
    std::vector<char> text;
    std::vector<char> symtab;
    std::vector<char> strtab;
    std::vector<char> rel_text;

    std::vector<Symbol> symbols;

    void ReadFileHeader();
    void ReadSectionHeaders();
    void ReadShstrtab();
    void ReadSections();

    ObjectFile(std::string filename)
    : file_name(filename), ifs(filename, std::ios::binary) {
        ReadFileHeader();
        ReadSectionHeaders();
        ReadShstrtab();
        ReadSections();
    }
};

class LibFile : public ObjectFile
{
  public:
    std::vector<char> dynsym;
    std::vector<char> dynstr;
    std::vector<char> dynamic;

    void ReadDynSections();
    LibFile(std::string filename)
    : ObjectFile(filename) {
        ReadDynSections();
    }
};