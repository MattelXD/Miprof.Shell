#include "Utils.h"
#include <sstream>

namespace utils {

std::vector<std::string> tokenize(const std::string &line) {
    
    std::vector<std::string> tokens;
    std::string cur;
    
    bool in_double = false;
    bool in_single = false;
    
    for (std::size_t i = 0; i < line.size(); ++i) {
    
        char c = line[i];
    
        if (c == '"' && !in_single) {
            in_double = !in_double;
            continue;
        }
    
        if (c == '\'' && !in_double) {
            in_single = !in_single;
            continue;
        }
    
        if (std::isspace((unsigned char)c) && !in_double && !in_single) {
    
            if (!cur.empty()) {
                tokens.push_back(cur);
                cur.clear();
            }
    
            continue;
        }
    
        cur.push_back(c);
    }
    
    if (!cur.empty()) tokens.push_back(cur);
    
    return tokens;
}

std::string join_args(const std::vector<std::string> &args, std::size_t start) {
    
    std::string out;
    
    for (std::size_t i = start; i < args.size(); ++i) {
    
        if (i > start) out += " ";
        out += args[i];
    }
    
    return out;
}

} // final de la funcion utils