#include <fstream>
#include <cassert>
#include <vector>

#include "object.h"

void ResolveSymbols(std::vector<ObjectFile>& files);
void WriteOutput(std::ofstream& ofs);

int main(int argc, char** argv) {
    assert(argc > 2);
    std::vector<ObjectFile> in_files;

    for(int i = 1; i < argc-1; ++i) {
        in_files.emplace_back(argv[i]);
    }

    std::ofstream ofs(argv[argc-1], std::ios::binary);

    ResolveSymbols(in_files);
    WriteOutput(ofs);


    return 0;
}