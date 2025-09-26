#ifndef SHELL_H
#define SHELL_H

#include <string>
#include <vector>

using namespace std;

class Shell {

public:

    Shell();
    void loop();

private:

    void handle_line(const string &line);
    void execute_simple_command(const vector<string> &args);
    void execute_piped_command(const vector<string> &segments);
    bool is_builtin_miprof(const vector<string> &args);
    int handle_miprof_builtin(const vector<string> &args);
};

#endif