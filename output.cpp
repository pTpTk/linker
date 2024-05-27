#include <fstream>
#include <vector>
#include <array>

#include "dynamic.h"

#define FH_SIZE 0x40
#define PH_SIZE 0x38

extern std::vector<char> texts;
extern std::vector<char> dynsym;
extern std::string dynstr;
extern std::vector<char> rela_plt;
extern std::vector<uint8_t> plt;
extern std::vector<uint64_t> got;
extern Dynamic dynamic;

namespace {

std::vector<char> file_header
{
    0x7F, 0x45, 0x4C, 0x46, 0x02, 0x01, 0x01, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x03, 0x00, 0x3E, 0x00, 0x01, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x38, 0x00,
    0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00 
};

int32_t& e_entry = (int32_t&)file_header[0x18];
int16_t& e_phnum = (int16_t&)file_header[0x38];

void WriteFileHeader(std::ofstream& ofs) {
    e_entry = 0x1000;
    e_phnum = 6;

    ofs.write(file_header.data(), file_header.size());
}

struct Program_Header
{
    std::vector<char> bin = std::vector<char>(PH_SIZE,0);
    uint32_t& p_type   = (uint32_t&)bin[0x00];
    uint32_t& p_flags  = (uint32_t&)bin[0x04];
    uint64_t& p_offset = (uint64_t&)bin[0x08];
    uint64_t& p_vaddr  = (uint64_t&)bin[0x10];
    uint64_t& p_paddr  = (uint64_t&)bin[0x18];
    uint64_t& p_filesz = (uint64_t&)bin[0x20];
    uint64_t& p_memsz  = (uint64_t&)bin[0x28];
    uint64_t& p_align  = (uint64_t&)bin[0x30];
};

std::vector<Program_Header> program_headers(6);
Program_Header& PHDR       = program_headers[0];
Program_Header& PH_INTERP  = program_headers[1];
Program_Header& PH_LOAD_0  = program_headers[2];
Program_Header& PH_LOAD_1  = program_headers[3];
Program_Header& PH_LOAD_2  = program_headers[4];
Program_Header& PH_DYNAMIC = program_headers[5];

void WriteInterp(std::ofstream& ofs) {
    std::string interp = "/lib64/ld-linux-x86-64.so.2";
    interp += '\0';
    int interp_size = interp.size();
    int ph_interp_offset = FH_SIZE + PH_SIZE * e_phnum;

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
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    };

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

    align(ofs, 8);

    dynamic.jmprel = ofs.tellp();
    dynamic.pltrelsz = rela_plt.size();
    ofs.write(rela_plt.data(), rela_plt.size());

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
    dynamic.pltgot = 0x2000 + (12 << 4);
    dynamic.generate();
    
    ofs.seekp(0x2000);
    ofs.write(dynamic.output.data(), dynamic.output.size());
    ofs.write((char*)got.data(), got.size() << 3);

    PH_DYNAMIC.p_type = 0x02;
    PH_DYNAMIC.p_offset = 0x2000;
    PH_DYNAMIC.p_vaddr  = 0x2000;
    PH_DYNAMIC.p_paddr  = 0x2000;
    PH_DYNAMIC.p_filesz = dynamic.output.size();
    PH_DYNAMIC.p_memsz  = dynamic.output.size();
    PH_DYNAMIC.p_flags  = 0x06;
    PH_DYNAMIC.p_align  = 0x04;

    uint load2_size = dynamic.output.size() + (got.size() << 3);

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
    PHDR.p_offset = FH_SIZE;
    PHDR.p_vaddr  = FH_SIZE;
    PHDR.p_paddr  = FH_SIZE;
    PHDR.p_filesz = PH_SIZE * e_phnum;
    PHDR.p_memsz  = PH_SIZE * e_phnum;
    PHDR.p_flags  = 0x04;
    PHDR.p_align  = 0x04;

    ofs.seekp(FH_SIZE);
    for(auto& ph : program_headers) {
        ofs.write(ph.bin.data(), PH_SIZE);
    }
}

} // namespace

void WriteOutput(std::ofstream& ofs) {

    WriteFileHeader(ofs);
    WriteInterp(ofs);
    WriteMisc(ofs);
    WriteText(ofs);
    WriteLoad2(ofs);
    WriteProgramHeaders(ofs);

}