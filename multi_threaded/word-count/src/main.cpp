#include <getopt.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <thread>

#include "word_count.h"

void print_usage() {
    std::cout << "Usage: ./word-count -f <file> -t <threads> [-lwc]\n"
              << "  -f <file>     Specifies the input file (required)\n"
              << "  -t <threads>  Number of threads (optional, default = CPU cores)\n"
              << "  -l            Count lines\n"
              << "  -w            Count words\n"
              << "  -c            Count characters\n";
}

int main(int argc, char* argv[]) {
    const char* file = nullptr;  
    int num_threads = std::thread::hardware_concurrency(); 

    WordCountFlags flags;  

    static option long_options[] = {
        {"file",    required_argument, 0, 'f'},
        {"threads", required_argument, 0, 't'},
        {"lines",   no_argument,       0, 'l'},
        {"words",   no_argument,       0, 'w'},
        {"chars",   no_argument,       0, 'c'},
        {0,         0,                 0,  0 }
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "f:t:lwc", long_options, nullptr)) != -1) {
        switch (opt) {
            case 'f': file = optarg; break;
            case 't': num_threads = std::stoi(optarg); break;
            case 'l': flags.lines = true; break;
            case 'w': flags.words = true; break;
            case 'c': flags.chars = true; break;
            case '?': print_usage(); return 1; 
        }
    }

    if (!file) {
        std::cerr << "Error: A file must be provided with -f <file>\n";
        print_usage();
        return 1;
    }

    if (!flags.lines && !flags.words && !flags.chars)
        flags.lines = flags.words = flags.chars = true;

    std::ifstream fh(file);
    if (!fh.is_open()) {
        std::cerr << "Error: Could not open file '" << file << "'\n";
        return 1;
    }

    std::cout << "Processing file: " << file << "\n";
    std::cout << "Using " << num_threads << " threads\n";
    
    std::vector<std::string> files;
    std::string line;
    while (getline(fh, line)) {
        files.push_back(line);
    }

    fh.close();  

    word_count(flags, files);  

    return 0;
}

