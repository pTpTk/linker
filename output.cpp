#include <fstream>
#include <vector>
#include <array>

extern std::vector<char> text;

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

void WriteFileHeader(std::ofstream& ofs) {
    e_type = 0x2;
    e_entry = 0x08049000;
    e_phnum = 2;

    ofs.write(file_header.data(), file_header.size());
}

void WritePHDR(std::ofstream& ofs) {
    Program_Header phdr;
    phdr.p_type   = 0x01;
    phdr.p_offset = 0x00;
    phdr.p_vaddr  = 0x08048000;
    phdr.p_paddr  = 0x08048000;
    phdr.p_filesz = 0x34 + 0x20 * e_phnum;
    phdr.p_memsz  = 0x34 + 0x20 * e_phnum;
    phdr.p_flags  = 0x04;
    phdr.p_align  = 0x1000;

    ofs.write(phdr.bin.data(), 0x20);
}

void WriteText(std::ofstream& ofs) {
    Program_Header ph_text;
    int text_size = text.size();
    ph_text.p_type   = 0x01;
    ph_text.p_offset = 0x1000;
    ph_text.p_vaddr  = 0x08048000 + 0x1000;
    ph_text.p_paddr  = 0x08048000 + 0x1000;
    ph_text.p_filesz = text_size;
    ph_text.p_memsz  = text_size;
    ph_text.p_flags  = 0x05;
    ph_text.p_align  = 0x1000;

    ofs.write(ph_text.bin.data(), 0x20);

    ofs.seekp(0x1000);
    ofs.write(text.data(), text_size);
}

} // namespace

void WriteOutput(std::ofstream& ofs) {

    WriteFileHeader(ofs);
    WritePHDR(ofs);
    WriteText(ofs);

}