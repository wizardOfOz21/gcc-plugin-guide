#include <string>
#include <fstream>
#include <vector>

using std::string;

void write_to_file(string filepath, const string& data);

string q(string str);

string subgraph(const string& str, string name);

string graph(const string& str, string name);

string cluster(string name);

string graph_edge(string left, string right);

string graph_arg(string key, string value);

string graph_args(std::vector<string> args);
