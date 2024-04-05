#include <iostream>
#include <fstream>
#include <cassert>
#include <vector>
#include <sstream>
#include <cstring>

#ifdef DEBUG
    #define D(...) printf(__VA_ARGS__)
#else
    #define D(...)
#endif

constexpr uint file_header_size = 0x34;

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

std::vector<char> file_header(file_header_size);
uint32_t e_shoff;  
uint16_t e_shnum;
uint16_t e_shstrndx;

void ReadFileHeader(std::ifstream& ifs) {
    ifs.read(file_header.data(), file_header_size);

    assert(file_header[0] == 0x7F);
    assert(file_header[1] == 0x45);
    assert(file_header[2] == 0x4C);
    assert(file_header[3] == 0x46);

    {
        D("file_header:\n");
        int i = 0;
        for(auto c : file_header) {
            D("%02x ", c);
            if(++i % 16 == 0) D("\n");
        }
        D("\n");
    }

    uint32_t* u32_ptr = (uint32_t*)file_header.data();
    uint16_t* u16_ptr = (uint16_t*)file_header.data();

    e_shoff    = u32_ptr[0x08];
    e_shnum    = u16_ptr[0x18];
    e_shstrndx = u16_ptr[0x19];

    D("e_shoff = %x, e_shnum = %d, e_shstrndx = %d\n",
        e_shoff, e_shnum, e_shstrndx);
}

std::vector<Section_Header> section_headers;

void ReadSectionHeaders(std::ifstream& ifs) {
    section_headers.resize(e_shnum);

    ifs.seekg(e_shoff);

    for(int i = 0; i < e_shnum; i++) {
        ifs.read(section_headers[i].bin.data(), 0x28);
    }
}

std::vector<char> shstrtab;

void ReadShstrtab(std::ifstream& ifs) {
    Section_Header& shstrtab_section_header 
        = section_headers[e_shstrndx];

    shstrtab.resize(shstrtab_section_header.sh_size);

    ifs.seekg(shstrtab_section_header.sh_offset);
    ifs.read(shstrtab.data(), shstrtab.size());
}

std::vector<Section> sections;
Section text;

void ReadSections(std::ifstream& ifs) {
    for(int j = 1; j < e_shnum; ++j) {
        auto& sh = section_headers[j];

        Section section;
        section.name = std::string(shstrtab.data() + sh.sh_name);
        section.alloc(sh.sh_size);

        ifs.seekg(sh.sh_offset);
        ifs.read(section.bin.data(), sh.sh_size);

        sections.push_back(section);

        {
            D("%s section:\n", section.name.c_str());
            int i = 0;
            for(auto& c : section.bin) {
                D("%02x ", (unsigned char)c);
                if(++i % 16 == 0) D("\n");
            }
            D("\n");
        }
    }

    for(auto& s : sections) {
        if(s.name == ".text") {
            text = s;
            break;
        }
    }
}

void ReadInput(std::ifstream& ifs) {
    ReadFileHeader(ifs);
    ReadSectionHeaders(ifs);
    ReadShstrtab(ifs);
    ReadSections(ifs);
}