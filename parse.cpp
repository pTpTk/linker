#include <iostream>
#include <fstream>
#include <cassert>
#include <vector>
#include <sstream>
#include <cstring>

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
    std::vector<char> bin;
    std::string name;

    void alloc(uint size) { bin.resize(size); }
};

void BuildSections(std::ifstream ifs){}

int main(int argc, char** argv) {
    assert(argc == 2);
    std::ifstream ifs(argv[1], std::ios::binary);

    std::vector<char> file_header(file_header_size);
    ifs.read(file_header.data(), file_header_size);

    assert(file_header[0] == 0x7F);
    assert(file_header[1] == 0x45);
    assert(file_header[2] == 0x4C);
    assert(file_header[3] == 0x46);

    printf("file_header:\n");
    int i = 0;
    for(auto c : file_header) {
        printf("%02x ", c);
        if(++i % 16 == 0) printf("\n");
    }
    printf("\n");

    uint32_t* u32_ptr = (uint32_t*)file_header.data();
    uint16_t* u16_ptr = (uint16_t*)file_header.data();

    uint32_t e_shoff    = u32_ptr[0x08];
    uint16_t e_shnum    = u16_ptr[0x18];
    uint16_t e_shstrndx = u16_ptr[0x19];

    printf("e_shoff = %x, e_shnum = %d, e_shstrndx = %d\n",
        e_shoff, e_shnum, e_shstrndx);

    std::vector<Section_Header> section_headers(e_shnum);
    Section_Header& shstrtab_section_header = section_headers[e_shstrndx];
    
    ifs.seekg(e_shoff);

    for(int i = 0; i < e_shnum; i++) {
        ifs.read(section_headers[i].bin.data(), 0x28);
    }

    printf("shstrtab section header:\n");
    i = 0;
    for(auto& c : shstrtab_section_header.bin) {
        printf("%02x ", c);
        if(++i % 16 == 0) printf("\n");
    }
    printf("\n");

    std::vector<char> shstrtab(shstrtab_section_header.sh_size);

    ifs.seekg(shstrtab_section_header.sh_offset);
    ifs.read(shstrtab.data(), shstrtab.size());

    printf("shstrtab section:\n");
    i = 0;
    for(auto& c : shstrtab) {
        printf("%02x ", c);
        if(++i % 16 == 0) printf("\n");
    }
    printf("\n");

    for(int j = 1; j < e_shnum; ++j) {
        auto& sh = section_headers[j];

        Section section;
        section.name = std::string(shstrtab.data() + sh.sh_name);
        section.alloc(sh.sh_size);

        ifs.seekg(sh.sh_offset);
        ifs.read(section.bin.data(), sh.sh_size);

        printf("%s section:\n", section.name.c_str());
        i = 0;
        for(auto& c : section.bin) {
            printf("%02x ", (unsigned char)c);
            if(++i % 16 == 0) printf("\n");
        }
        printf("\n");
    }

    return 0;
}