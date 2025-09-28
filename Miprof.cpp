#include "Miprof.h"
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <csignal>
#include <chrono>

using namespace std;

int Miprof::run(const string &mode,
                const vector<string> &args,
                const string &outFile,
                int timeoutSeconds) {
    if (args.empty()) {
        cerr << "miprof: no se proporcionó comando\n";
        return -1;
    }

    pid_t pid;
    struct rusage usage;
    int status;
    auto start = chrono::steady_clock::now();

    pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }

    if (pid == 0) {
        // Proceso hijo: ejecutar el comando
        vector<char*> argv;
        for (const auto &arg : args) {
            argv.push_back(const_cast<char*>(arg.c_str()));
        }
        argv.push_back(nullptr);

        execvp(argv[0], argv.data());
        perror("execvp");
        _exit(127);
    }

    // Proceso padre: medir tiempos, esperar con timeout si aplica
    bool killed = false;
    if (timeoutSeconds > 0) {
        auto deadline = chrono::steady_clock::now() + chrono::seconds(timeoutSeconds);

        while (true) {
            pid_t res = wait4(pid, &status, WNOHANG, &usage);
            if (res == pid) break; // terminó

            if (chrono::steady_clock::now() >= deadline) {
                kill(pid, SIGKILL);
                wait4(pid, &status, 0, &usage);
                killed = true;
                break;
            }
            usleep(100000); // dormir 100ms antes de volver a checkear
        }
    } else {
        wait4(pid, &status, 0, &usage);
    }

    auto end = chrono::steady_clock::now();
    chrono::duration<double> real_dur = end - start;

    double real_sec = real_dur.count();
    double user_sec = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1e6;
    double sys_sec  = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1e6;
    long max_rss    = usage.ru_maxrss; // en KB en Linux

    if (killed) {
        cerr << "miprof: proceso excedió el tiempo máximo y fue terminado\n";
    }

    // Mostrar resultados en pantalla
    format_and_print_result(args, real_sec, user_sec, sys_sec, max_rss);

    // Guardar en archivo si corresponde
    if (mode == "ejecsave" && !outFile.empty()) {
        append_result_to_file(outFile, args, real_sec, user_sec, sys_sec, max_rss);
    }

    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

void Miprof::format_and_print_result(const vector<string>& cmd,
                                     double real_sec,
                                     double user_sec,
                                     double sys_sec,
                                     long max_rss) {
    cout << "Comando: ";
    for (auto &c : cmd) cout << c << " ";
    cout << "\n";

    cout << "Tiempo real: " << real_sec << " s\n";
    cout << "Tiempo usuario: " << user_sec << " s\n";
    cout << "Tiempo sistema: " << sys_sec << " s\n";
    cout << "Máx memoria residente: " << max_rss << " KB\n";
    cout << "-----------------------------------\n";
}

void Miprof::append_result_to_file(const string &filename,
                                   const vector<string>& cmd,
                                   double real_sec,
                                   double user_sec,
                                   double sys_sec,
                                   long max_rss) {
    ofstream ofs(filename, ios::app);
    if (!ofs) {
        cerr << "miprof: no se pudo abrir archivo " << filename << "\n";
        return;
    }

    ofs << "Comando: ";
    for (auto &c : cmd) ofs << c << " ";
    ofs << "\n";

    ofs << "Tiempo real: " << real_sec << " s\n";
    ofs << "Tiempo usuario: " << user_sec << " s\n";
    ofs << "Tiempo sistema: " << sys_sec << " s\n";
    ofs << "Máx memoria residente: " << max_rss << " KB\n";
    ofs << "-----------------------------------\n";
}