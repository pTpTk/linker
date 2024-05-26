#include <fstream>
#include <cassert>
#include <vector>

#include "object.h"

void ResolveSymbols(std::vector<ObjectFile>& files);
void ProcessSharedLibs(std::vector<LibFile>& libs);
void WriteOutput(std::ofstream& ofs);

int main(int argc, char** argv) {
    assert(argc > 2);
    std::vector<ObjectFile> obj_files;
    std::vector<LibFile> lib_files;

    int i = 1;
    for(; i < argc-1; ++i) {
        if(argv[i][0] == '-') break;
        obj_files.emplace_back(argv[i]);
    }

    for(; i < argc-1; ++i) {
        assert(argv[i][0] == '-');
        assert(argv[i][1] == 'l');
        std::string lib_name(argv[i]+2);
        lib_name = "lib" + lib_name + ".so";
        lib_files.emplace_back(lib_name);
    }

    // focus on at most 1 lib for now
    assert(lib_files.size() < 2);

    std::ofstream ofs(argv[argc-1], std::ios::binary);

    ResolveSymbols(obj_files);
    ProcessSharedLibs(lib_files);
    WriteOutput(ofs);


    return 0;
}