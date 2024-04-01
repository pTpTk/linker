#include <fstream>
#include <cassert>

void ReadInput(std::ifstream& ifs);

int main(int argc, char** argv) {
    assert(argc == 2);
    std::ifstream ifs(argv[1], std::ios::binary);

    ReadInput(ifs);

    return 0;
}