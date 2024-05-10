#include "print.hpp"

string to_graph_bb() {

}

void write_to_file(string filepath, const string& data) {
    std::ofstream file(filepath);
    file << data;
    file.close();
}

