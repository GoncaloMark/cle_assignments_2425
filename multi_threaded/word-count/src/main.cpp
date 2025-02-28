#include <getopt.h>
#include <iostream>
#include <fstream>

#include "word_count.h"

static option long_options[] = {
    {"file", required_argument, 0, 'f'},
    {"chunk_size", required_argument, 0, 'z'},
    {"threads", required_argument, 0, 't'},
    {"lines",   no_argument, 0,  0 },
    {"words",   no_argument, 0,  0 },
    {"chars",   no_argument, 0,  0 },
    {0,         0,           0,  0 }
};

void print_usage() {
    std::cerr << "Usage: ./word-count -f <file> -c <chunk_size> -t <threads> [-lwc]\n"
              << "  -f, --file <file>         Input file (required)\n"
              << "  -z, --chunk_size <size>   Chunk size (required)\n"
              << "  -t, --threads <num>       Number of threads (required)\n"
              << "  -l, --lines               Count lines\n"
              << "  -w, --words               Count words\n"
              << "  -c, --chars               Count characters\n";
}

int main(int argc, char* argv[])
{
    WordCountFlags flags;
    std::string file;
    size_t chunk_size = -1;
    int num_threads = -1;

    while(1) {
        int option_index = 0;
        int c = getopt_long(argc, argv, "f:z:t:lwc", long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
            case 'f': file = optarg; break;
            case 'z': chunk_size = std::atoi(optarg); break;
            case 't': num_threads =std::atoi(optarg); break;
            case 'l': flags.lines = true; break;
            case 'w': flags.words = true; break;
            case 'c': flags.chars = true; break;
            case '?':
                // getopt_long already printed an error message
                print_usage();
                return 1;
                //break;
        }// end switch
    }// end while(1)

    if (!flags.lines && ! flags.words && !flags.chars)
        flags.lines = flags.words = flags.chars = true;

    if (file.empty() || chunk_size <= 0 || num_threads <= 0){
        std::cerr << "Error: Missing required arguments!\n";
        print_usage();
        return 1;
    }

    size_t chunk_size_mb = chunk_size * (1024 * 1024);
    std::cout << "Chunk size: " << chunk_size_mb << "\n";
    std::cout << "Number of Threads: " << num_threads << "\n";

    std::vector<std::string> files;
    std::ifstream infile(file);  

    if(!infile.is_open()) {
        std::cerr << "Error: Unable to open file list: " << file << "\n";
        return 1;
    } else {
        std::string line;
        while (getline(infile, line)){
            files.push_back(line);
        }
    }

    if (files.empty()){
        std::cerr << "Error: No files listed inside" << file << "\n";
    }

    /*
    // check if there are any files
    if (optind < argc) {
        std::string fname = argv[optind];
        std::ifstream file(fname);
        // one file per line
        // enter your code here
        std::string line;
        while(getline(file, line)){
            files.push_back(line);
        }
    } else {
        std::cout << "No files provided" << std::endl;
        return 1;
    } */

    word_count(flags, files);

    return 0;
}
