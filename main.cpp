#include <fstream>
#include <cassert>

void ReadInput(std::ifstream& ifs);
void ResolveSymbols();
void WriteOutput(std::ofstream& ofs);

int main(int argc, char** argv) {
    assert(argc == 3);
    std::ifstream ifs(argv[1], std::ios::binary);
    std::ofstream ofs(argv[2], std::ios::binary);

    ReadInput(ifs);
    ResolveSymbols();
    WriteOutput(ofs);

    return 0;
}