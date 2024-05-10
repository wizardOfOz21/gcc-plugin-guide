#include "print.hpp"
#include <iostream>

int main() {
    write_to_file("build/out", "Hello, world!");
    std::cout << subgraph("1->2", "cluster_1") << std::endl;
    return 0;
}