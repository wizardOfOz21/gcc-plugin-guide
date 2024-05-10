#include <sstream>
#include "print.hpp"

string q(string str) {
    return "\"" + str + "\"";
}

string graph_arg(string key, string value) {
    return key + "=" + value;
}

string graph_args(std::vector<string> args) {
    string res = "";
    res += "[";
    for (string arg : args) {
        res += arg + ',';
    }
    res.pop_back();
    res += "]";
    return res;
}

string cluster(string name) {
    return "cluster_"+name;
}

string subgraph(const string& str, string name) {
    return "subgraph " + cluster(name) + " {\n" + str + "\nlabel=" + q(name) +"\n}";
}

string graph(const string& str, string name) {
    std::stringstream res;

    auto params = {graph_arg("compound", "true")};

    res << "digraph " << name << " {" << std::endl;

    for (auto param : params) {
        res << param << std::endl;
    }

    res << str << std::endl;

    res << "}" << std::endl;

    return res.str();
}

string graph_edge(string left, string right) {
    return left + "->" + right;
}

void write_to_file(string filepath, const string& data) {
    std::ofstream file(filepath);
    file << data;
    file.close();
}

