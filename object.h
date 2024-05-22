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
    std::vector<char> bin = std::vector<char>(0x40,0);
    int32_t& sh_name    = (int32_t&)bin[0x00];
    int32_t& sh_type    = (int32_t&)bin[0x04];
    int64_t& sh_flags   = (int64_t&)bin[0x08];
    int64_t& sh_addr    = (int64_t&)bin[0x10];
    int64_t& sh_offset  = (int64_t&)bin[0x18];
    int64_t& sh_size    = (int64_t&)bin[0x20];
    int32_t& sh_link    = (int32_t&)bin[0x28];
    int32_t& sh_info    = (int32_t&)bin[0x2C];
    int64_t& sh_align   = (int64_t&)bin[0x30];
    int64_t& sh_entsize = (int64_t&)bin[0x38];
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

    std::vector<char> file_header = std::vector<char>(0x40);
    uint64_t& e_shoff    = (uint64_t&)file_header[0x28];
    uint16_t& e_shnum    = (uint16_t&)file_header[0x3C];
    uint16_t& e_shstrndx = (uint16_t&)file_header[0x3E];

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