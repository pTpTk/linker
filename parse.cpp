#include <iostream>
#include <fstream>
#include <cassert>
#include <vector>

constexpr uint file_header_size = 0x34;

struct Section_Header
{
    int sh_name;
    int sh_type;
    int sh_flags;
    int sh_addr;
    int sh_offset;
    int sh_size;
    int sh_link;
    int sh_info;
    int sh_align;
    int sh_entsize;
};

int main(int argc, char** argv) {
    assert(argc == 2);
    std::ifstream ifs(argv[1], std::ios::binary);

    std::vector<char> file_header(file_header_size);
    ifs.read(file_header.data(), file_header_size);

    assert(file_header[0] == 0x7F);
    assert(file_header[1] == 0x45);
    assert(file_header[2] == 0x4C);
    assert(file_header[3] == 0x46);

    int i = 0;

    for(auto c : file_header) {
        printf("%02x ", c);
        if(++i % 16 == 0) printf("\n");
    }

    printf("\n");

    uint32_t* u32_ptr = (uint32_t*)file_header.data();
    uint16_t* u16_ptr = (uint16_t*)file_header.data();

    uint32_t section_table_offset = u32_ptr[8];
    uint16_t section_count = u16_ptr[0x18];
    uint16_t shstrtab_index = u16_ptr[0x19];

    printf("section_table_offset = %x, section_count = %d, shstrtab_index = %d\n",
        section_table_offset, section_count, shstrtab_index);

    Section_Header shstrtab_section_header;
    
    ifs.seekg(section_table_offset);
    ifs.seekg(shstrtab_index * 0x28, std::ios_base::cur);

    ifs.read((char*)&shstrtab_section_header, 0x28);

    for(auto c : file_header) {
        printf("%02x ", c);
        if(++i % 16 == 0) printf("\n");
    }

    printf("\n");

    
    return 0;
}