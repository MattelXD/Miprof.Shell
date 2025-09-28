#include "shell.h"
#include "Utils.h"
#include "Miprof.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstring>

using namespace utils;

Shell::Shell() {}

static inline string trim(const string &s) {
    size_t a = s.find_first_not_of(" \t\n\r");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\n\r");
    return s.substr(a, b - a + 1);
}

void Shell::loop() {
    string line;
    while (true) {
        cout << "mishell:$ ";
        cout.flush();

        if (!std::getline(cin, line)) {
            cout << "\n";
            break;
        }

        line = trim(line);

        if (line.empty()) continue;

        handle_line(line);
    }
}

void Shell::handle_line(const string &line) {
    vector<string> segments;
    size_t start = 0;
    for (size_t i = 0; i <= line.size(); ++i) {
        if (i == line.size() || line[i] == '|') {
            string seg = trim(line.substr(start, i - start));
            if (!seg.empty()) segments.push_back(seg);
            start = i + 1;
        }
    }

    if (segments.empty()) return;

    if (segments.size() == 1) {
        vector<string> args = tokenize(segments[0]);
        if (args.empty()) return;

        if (args[0] == "exit") {
            exit(0);
        }

        if (is_builtin_miprof(args)) {
            handle_miprof_builtin(args);
            return;
        }

        execute_simple_command(args);
    } else {
        execute_piped_command(segments);
    }
}

void Shell::execute_simple_command(const vector<string> &args) {
    vector<char*> argv;
    for (const auto &a : args) argv.push_back(const_cast<char*>(a.c_str()));
    argv.push_back(nullptr);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }
    if (pid == 0) {
        execvp(argv[0], argv.data());
        perror("execvp");
        _exit(127);
    } else {
        int status;
        if (waitpid(pid, &status, 0) < 0) {
            perror("waitpid");
        }
    }
}

void Shell::execute_piped_command(const vector<string> &segments) {
    size_t n = segments.size();
    vector<int> pids;
    int last_read_fd = -1;

    for (size_t i = 0; i < n; ++i) {
        vector<string> args = tokenize(segments[i]);
        if (args.empty()) continue;

        vector<char*> argv;
        for (const auto &a : args) argv.push_back(const_cast<char*>(a.c_str()));
        argv.push_back(nullptr);

        int pipefd[2] = {-1, -1};
        if (i != n - 1) {
            if (pipe(pipefd) < 0) {
                perror("pipe");
                return;
            }
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return;
        }

        if (pid == 0) {
            if (last_read_fd != -1) {
                if (dup2(last_read_fd, STDIN_FILENO) < 0) {
                    perror("dup2 stdin");
                    _exit(127);
                }
            }
            if (pipefd[1] != -1) {
                if (dup2(pipefd[1], STDOUT_FILENO) < 0) {
                    perror("dup2 stdout");
                    _exit(127);
                }
            }

            if (pipefd[0] != -1) close(pipefd[0]);
            if (pipefd[1] != -1) close(pipefd[1]);
            if (last_read_fd != -1) close(last_read_fd);

            execvp(argv[0], argv.data());
            perror("execvp");
            _exit(127);
        } else {
            pids.push_back(pid);
            if (pipefd[1] != -1) close(pipefd[1]);
            if (last_read_fd != -1) close(last_read_fd);
            last_read_fd = (pipefd[0] != -1) ? pipefd[0] : -1;
        }
    }

    for (int pid : pids) {
        int status;
        if (waitpid(pid, &status, 0) < 0) perror("waitpid");
    }
}

bool Shell::is_builtin_miprof(const vector<string> &args) {
    if (args.empty()) return false;
    return args[0] == "miprof";
}

int Shell::handle_miprof_builtin(const vector<string> &args) {
    if (args.size() < 2) {
        cerr << "Uso: miprof [ejec|ejecsave archivo|ejecutar maxtiempo] comando args...\n";
        return -1;
    }

    string mode = args[1];
    if (mode == "ejec") {
        vector<string> cmd(args.begin() + 2, args.end());
        return Miprof::run("ejec", cmd);
    } else if (mode == "ejecsave") {
        if (args.size() < 4) {
            cerr << "Uso: miprof ejecsave archivo comando args...\n";
            return -1;
        }
        string filename = args[2];
        vector<string> cmd(args.begin() + 3, args.end());
        return Miprof::run("ejecsave", cmd, filename);
    } else if (mode == "ejecutar") {
        if (args.size() < 4) {
            cerr << "Uso: miprof ejecutar maxtiempo comando args...\n";
            return -1;
        }
        int timeout = stoi(args[2]);
        vector<string> cmd(args.begin() + 3, args.end());
        return Miprof::run("ejecutar", cmd, "", timeout);
    } else {
        cerr << "Modo no reconocido para miprof\n";
        return -1;
    }
}

