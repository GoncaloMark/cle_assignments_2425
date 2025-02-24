#include <iostream>
#include <iomanip> 
#include <fstream>
#include <map>
#include <chrono>

#include "../include/datastructures.hpp"

int main(int argc, char* argv[])
{
    const char* file = "measurements.txt";
    if (argc > 1){
        file = argv[1];
    }
    std::ifstream fh(file);
    if (not fh.is_open()){
        std::cerr << "Unable to open '" << file << "'" << '\n';
        std::cerr << "Please ensure the file exists and you have permission to read it." << '\n';
        return 1;
    }

    auto start_time = std::chrono::high_resolution_clock::now();

    std::string line;
    std::map<std::string, data_t> store;

    while(getline(fh, line)) {
        size_t idx = line.find(';');
        if (idx == std::string::npos) {
            std::cerr << "Skipping malformed line (missing ';'): " << line << '\n';
            continue;
        }

        char *str = line.data();
        str[idx] = '\0';

        try {
            auto& data = store[str];
            str = str + idx + 1;
            float value = std::stof(str);

            data.sum += value;
            data.count += 1;

            if (value > data.max) {
                data.max = value;
            }

            if (value < data.min) {
                data.min = value;
            }

        } catch (const std::invalid_argument& e) {
            std::cerr << "Invalid value (couldn't conver to float), skipping line: " << line << '\n';
        } catch (const std::out_of_range& e) {
            std::cerr << "Value out of range (couldn't conver to float), skipping line: " << line << '\n';
        }
    }

    for (const auto& entry : store) {
        std::cout << std::fixed << std::setprecision(1) << entry.first << ": avg=" << entry.second.sum / entry.second.count << " min=" << entry.second.min << " max=" << entry.second.max  << '\n';
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "Execution time: " << duration.count() << " milliseconds" << std::endl;

    // Always close the file when done
    fh.close();

    return 0;
}
