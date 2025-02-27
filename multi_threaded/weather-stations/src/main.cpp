#include <iostream>
#include <iomanip>
#include <fstream>
#include <map>
#include <chrono>
#include <getopt.h>
#include <cstring>
#include <thread>
#include <vector>

#include "../include/datastructures.hpp"

void print_usage()
{
    std::cout << "Use: ./cle-ws -f <file> -f <file2> -t <threads>\n"
              << "  -f <file>: Specifies the input file (required and can be used multiple times)\n"
              << "  -t <threads>: Number of threads (optional)\n";
}

int main(int argc, char *argv[])
{
    std::vector<std::string> files;
    // int num_threads = 1; // Base: 1 Thread
    int num_threads = std::thread::hardware_concurrency();

    if (argc < 2)
    {
        std::cerr << "Error: Missing arguments.\n";
        print_usage();
        return 1;
    }

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-f") == 0 && i + 1 < argc)
        {
            files.push_back(argv[i + 1]);
            i++;
        }
        else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc)
        {
            num_threads = std::stoi(argv[i + 1]);
            i++;
        }
    }

    if (files.empty())
    {
        files.push_back("measurements.txt");
    }

    std::vector<std::string> valid_files;
    for (const auto &file : files)
    {
        std::ifstream fh(file);
        if (!fh.is_open())
        {
            std::cerr << "Unable to open '" << file << "', skipping..." << '\n';
        }
        else
        {
            valid_files.push_back(file);
            fh.close();
        }
    }

    if (valid_files.empty())
    {
        std::cerr << "No valid input files. Exiting program.\n";
        return 1;
    }

    for (const auto &f : valid_files)
    {
        std::cout << "  - " << f << std::endl;
    }

    std::cout << "Using " << num_threads << " threads to process files:\n ";


    /*
    const char* file = "measurements.txt";
    if (argc > 1){
        file = argv[1];
    }
    std::ifstream fh(file);
    if (not fh.is_open()){
        std::cerr << "Unable to open '" << file << "'" << '\n';
        std::cerr << "Please ensure the file exists and you have permission to read it." << '\n';
        return 1;
    }*/

    auto start_time = std::chrono::high_resolution_clock::now();

    // std::string line;
    std::map<std::string, data_t> store;

    for (const auto &file : valid_files)
    {
        std::ifstream fh(file);
        std::string line;
        while (getline(fh, line))
        {
            size_t idx = line.find(';');
            if (idx == std::string::npos)
            {
                std::cerr << "Skipping malformed line (missing ';'): " << line << '\n';
                continue;
            }

            char *str = line.data();
            str[idx] = '\0';

            try
            {
                auto &data = store[str];
                str = str + idx + 1;
                float value = std::stof(str);

                data.sum += value;
                data.count += 1;

                if (value > data.max)
                {
                    data.max = value;
                }

                if (value < data.min)
                {
                    data.min = value;
                }
            }
            catch (const std::invalid_argument &e)
            {
                std::cerr << "Invalid value (couldn't conver to float), skipping line: " << line << '\n';
            }
            catch (const std::out_of_range &e)
            {
                std::cerr << "Value out of range (couldn't conver to float), skipping line: " << line << '\n';
            }
        }

        // Always close the file when done
        fh.close();
    }

    for (const auto &entry : store)
    {
        std::cout << std::fixed << std::setprecision(1) << entry.first << ": avg=" << entry.second.sum / entry.second.count << " min=" << entry.second.min << " max=" << entry.second.max << '\n';
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "Execution time: " << duration.count() << " milliseconds" << std::endl;

    // Always close the file when done
    // fh.close();

    return 0;
}
