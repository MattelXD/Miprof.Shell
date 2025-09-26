#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

using namespace std;

namespace utils {

    // Tokeniza una línea en argumentos, respetando comillas "..." y '...'
    vector<string> tokenize(const string &line);

    // Join vector<string> to single string with spaces (for saving commands)
    string join_args(const vector<string> &args, size_t start = 0);
}

#endif