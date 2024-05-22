#include <fstream>
#include <vector>
#include <array>

#include "dynamic.h"

extern std::vector<char> texts;
extern std::vector<char> dynsym;
extern std::string dynstr;
extern std::vector<char> rel_dyn;
extern std::vector<char> rel_plt;
extern std::vector<uint8_t> plt;
extern std::vector<uint> got;
extern Dynamic dynamic;

namespace {

std::vector<char> file_header
{
    0x7F, 0x45, 0x4C, 0x46, 0x01, 0x01, 0x01, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x03, 0x00, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x34, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x34, 0x00, 0x20, 0x00, 0x00, 0x00, 0x28, 0x00, 
    0x00, 0x00, 0x00, 0x00 
};

int16_t& e_type  = (int16_t&)file_header[0x10];
int32_t& e_entry = (int32_t&)file_header[0x18];
int16_t& e_phnum = (int16_t&)file_header[0x2C];

void WriteFileHeader(std::ofstream& ofs) {
    e_type = 0x3;
    e_entry = 0x1000;
    e_phnum = 6;

    ofs.write(file_header.data(), file_header.size());
}

struct Program_Header
{
    std::vector<char> bin = std::vector<char>(0x20,0);
    int& p_type   = (int&)bin[0x00];
    int& p_offset = (int&)bin[0x04];
    int& p_vaddr  = (int&)bin[0x08];
    int& p_paddr  = (int&)bin[0x0C];
    int& p_filesz = (int&)bin[0x10];
    int& p_memsz  = (int&)bin[0x14];
    int& p_flags  = (int&)bin[0x18];
    int& p_align  = (int&)bin[0x1C];
};

std::vector<Program_Header> program_headers(6);
Program_Header& PHDR       = program_headers[0];
Program_Header& PH_INTERP  = program_headers[1];
Program_Header& PH_LOAD_0  = program_headers[2];
Program_Header& PH_LOAD_1  = program_headers[3];
Program_Header& PH_LOAD_2  = program_headers[4];
Program_Header& PH_DYNAMIC = program_headers[5];

void WriteInterp(std::ofstream& ofs) {
    std::vector<char> interp = 
    {
        0x2F, 0x6C, 0x69, 0x62, 0x2F, 0x6C, 0x64, 0x2D, 
        0x6C, 0x69, 0x6E, 0x75, 0x78, 0x2E, 0x73, 0x6F, 
        0x2E, 0x32, 0x00
    };
    int interp_size = interp.size();
    int ph_interp_offset = 0x34 + 0x20 * e_phnum;

    ofs.seekp(ph_interp_offset);
    ofs.write(interp.data(), interp_size);

    PH_INTERP.p_type   = 0x03;
    PH_INTERP.p_offset = ph_interp_offset;
    PH_INTERP.p_vaddr  = ph_interp_offset;
    PH_INTERP.p_paddr  = ph_interp_offset;
    PH_INTERP.p_filesz = interp_size;
    PH_INTERP.p_memsz  = interp_size;
    PH_INTERP.p_flags  = 0x04;
    PH_INTERP.p_align  = 0x01;
}

void align(std::ofstream& ofs, int i) {
    uint pos = ofs.tellp();
    while(pos % i != 0) {
        pos++;
    }
    ofs.seekp(pos);
}

void WriteMisc(std::ofstream& ofs) {
    std::vector<char> empty_gnu_hash = 
    {
        0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    int gnu_hash_offset = 0x34 + 0x20 * e_phnum + 0x14;
    ofs.seekp(gnu_hash_offset);

    align(ofs, 4);

    dynamic.gnu_hash = ofs.tellp();
    ofs.write(empty_gnu_hash.data(), empty_gnu_hash.size());

    align(ofs, 4);
    
    dynamic.symtab = ofs.tellp();
    ofs.write(dynsym.data(), dynsym.size());

    align(ofs, 4);

    dynamic.strtab = ofs.tellp();
    dynamic.strsz = dynstr.size();
    ofs << dynstr;

    align(ofs, 4);

    dynamic.rel = ofs.tellp();
    dynamic.relsz = rel_dyn.size();
    ofs.write(rel_dyn.data(), rel_dyn.size());

    align(ofs, 4);

    dynamic.jmprel = ofs.tellp();
    dynamic.pltrelsz = rel_plt.size();
    ofs.write(rel_plt.data(), rel_plt.size());

    align(ofs, 4);

    int load_size = ofs.tellp();

    PH_LOAD_0.p_type   = 0x01;
    PH_LOAD_0.p_offset = 0x00;
    PH_LOAD_0.p_vaddr  = 0x00;
    PH_LOAD_0.p_paddr  = 0x00;
    PH_LOAD_0.p_filesz = load_size;
    PH_LOAD_0.p_memsz  = load_size;
    PH_LOAD_0.p_flags  = 0x04;
    PH_LOAD_0.p_align  = 0x1000;
}

void WriteText(std::ofstream& ofs) {
    ofs.seekp(0x1000);
    ofs.write(texts.data(), texts.size());
    align(ofs, 0x10);
    ofs.write((char*)plt.data(), plt.size());

    int text_size = (int)ofs.tellp();
    text_size -= 0x1000;

    PH_LOAD_1.p_type   = 0x01;
    PH_LOAD_1.p_offset = 0x1000;
    PH_LOAD_1.p_vaddr  = 0x1000;
    PH_LOAD_1.p_paddr  = 0x1000;
    PH_LOAD_1.p_filesz = text_size;
    PH_LOAD_1.p_memsz  = text_size;
    PH_LOAD_1.p_flags  = 0x05;
    PH_LOAD_1.p_align  = 0x1000;
}

void WriteLoad2(std::ofstream& ofs) {
    dynamic.pltgot = 0x2000 + (17 << 3);
    dynamic.generate();
    
    ofs.seekp(0x2000);
    ofs.write(dynamic.output.data(), dynamic.output.size());
    ofs.write((char*)got.data(), got.size() << 2);

    PH_DYNAMIC.p_type = 0x02;
    PH_DYNAMIC.p_offset = 0x2000;
    PH_DYNAMIC.p_vaddr  = 0x2000;
    PH_DYNAMIC.p_paddr  = 0x2000;
    PH_DYNAMIC.p_filesz = 17 << 3;
    PH_DYNAMIC.p_memsz  = 17 << 3;
    PH_DYNAMIC.p_flags  = 0x06;
    PH_DYNAMIC.p_align  = 0x04;

    uint load2_size = dynamic.output.size() + (got.size() << 2);

    PH_LOAD_2.p_type = 0x01;
    PH_LOAD_2.p_offset = 0x2000;
    PH_LOAD_2.p_vaddr  = 0x2000;
    PH_LOAD_2.p_paddr  = 0x2000;
    PH_LOAD_2.p_filesz = load2_size;
    PH_LOAD_2.p_memsz  = load2_size;
    PH_LOAD_2.p_flags  = 0x06;
    PH_LOAD_2.p_align  = 0x1000;
}

void WriteProgramHeaders(std::ofstream& ofs) {
    PHDR.p_type   = 0x06;
    PHDR.p_offset = 0x34;
    PHDR.p_vaddr  = 0x34;
    PHDR.p_paddr  = 0x34;
    PHDR.p_filesz = 0x20 * e_phnum;
    PHDR.p_memsz  = 0x20 * e_phnum;
    PHDR.p_flags  = 0x04;
    PHDR.p_align  = 0x04;

    ofs.seekp(0x34);
    for(auto& ph : program_headers) {
        ofs.write(ph.bin.data(), 0x20);
    }
}

} // namespace

void WriteOutput(std::ofstream& ofs) {

    // WriteFileHeader(ofs);
    // WriteInterp(ofs);
    // WriteMisc(ofs);
    WriteText(ofs);
    // WriteLoad2(ofs);
    // WriteProgramHeaders(ofs);

}