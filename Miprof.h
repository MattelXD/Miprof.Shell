#ifndef MIPROF_H
#define MIPROF_H

#include <string>
#include <vector>

using namespace std;

class Miprof {
public:

    // Ejecuta miprof con "ejec" o "ejecsave <file>".
    static int run(const string &mode,
                   const vector<string> &args,
                   const string &outFile = "",
                   int timeoutSeconds = 0);

private:
    static void format_and_print_result(const vector<string>& cmd,
                                        double real_sec,
                                        double user_sec,
                                        double sys_sec,
                                        long max_rss);

    static void append_result_to_file(const string &filename,
                                      const vector<string>& cmd,
                                      double real_sec,
                                      double user_sec,
                                      double sys_sec,
                                      long max_rss);
};

#endif