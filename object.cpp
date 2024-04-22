#include "object.h"

void ObjectFile::ReadFileHeader() {
    ifs.read(file_header.data(), 0x34);

    assert(file_header[0] == 0x7F);
    assert(file_header[1] == 0x45);
    assert(file_header[2] == 0x4C);
    assert(file_header[3] == 0x46);

    {
        D("file_header:\n");
        int i = 0;
        for(unsigned char c : file_header) {
            D("%02x ", c);
            if(++i % 16 == 0) D("\n");
        }
        D("\n");
    }

    D("e_shoff = %x, e_shnum = %d, e_shstrndx = %d\n",
        e_shoff, e_shnum, e_shstrndx);
}

void ObjectFile::ReadSectionHeaders() {
    section_headers.resize(e_shnum);

    ifs.seekg(e_shoff);

    for(int i = 0; i < e_shnum; i++) {
        ifs.read(section_headers[i].bin.data(), 0x28);
    }
}

void ObjectFile::ReadShstrtab() {
    Section_Header& shstrtab_section_header 
        = section_headers[e_shstrndx];

    shstrtab.resize(shstrtab_section_header.sh_size);

    ifs.seekg(shstrtab_section_header.sh_offset);
    ifs.read(shstrtab.data(), shstrtab.size());
}

void ObjectFile::ReadSections() {
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
            text = s.bin;
            continue;
        }
        if(s.name == ".symtab") {
            symtab = s.bin;
            continue;
        }
        if(s.name == ".strtab") {
            strtab = s.bin;
            continue;
        }
        if(s.name == ".rel.text") {
            rel_text = s.bin;
            continue;
        }
    }
}

void LibFile::ReadDynSections() {
    for(auto& s : sections) {
        if(s.name == ".dynsym") {
            dynsym = s.bin;
            continue;
        }
        if(s.name == ".dynstr") {
            dynstr = s.bin;
            continue;
        }
        if(s.name == ".dynamic") {
            dynamic = s.bin;
            continue;
        }
    }
}