#include <iomanip>
#include <fstream>
#include <map>
#include <chrono>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h> 

#include "threading.hpp"
#include "../include/datastructures.hpp"

//constexpr size_t CHUNK_SIZE = 2 * (1024 * 1024); // 8MB chunk size
size_t CHUNK_SIZE = -1;
int num_threads = -1;
const char* file = "measurements.txt";

thread_local std::map<std::string, data_t> local_store;

void process_chunk(const char* start, const char* end) {
    const char* ptr = start;
    while (ptr < end) {
        const char* line_end = static_cast<const char*>(memchr(ptr, '\n', end - ptr));
        if (!line_end) break;

        const char* semicolon = static_cast<const char*>(memchr(ptr, ';', line_end - ptr));
        if (!semicolon) {
            std::cerr << "Skipping malformed line (missing ';'): " << std::string(ptr, line_end - ptr) << '\n';
        } else {
            try {
                std::string key(ptr, semicolon - ptr);
                float value = std::stof(std::string(semicolon + 1, line_end - (semicolon + 1)));

                auto& data = local_store[key];
                data.sum += value;
                data.count += 1;
                data.max = std::max(data.max, value);
                data.min = std::min(data.min, value);

            } catch (const std::exception& e) {
                std::cerr << "Skipping invalid line: " << std::string(ptr, line_end - ptr) << '\n';
            }
        }
        ptr = line_end + 1;  // go next line
    }
}

static option long_options[] = {
    {"file", required_argument, 0, 'f'},
    {"chunk_size", required_argument, 0, 'z'},
    {"threads", required_argument, 0, 't'},
    {0,         0,           0,  0 }
};

void print_usage() {
    std::cerr << "Usage: ./word-count -f <file> -z <chunk_size> -t <threads>\n"
              << "  -f, --file <file>         Input file (required)\n"
              << "  -z, --chunk_size <size>  Chunk size in MB (required)\n"
              << "  -t, --threads <num>       Number of threads (required)\n";
}

int main(int argc, char* argv[]) {
    int opt;
    while (1){
        int option_index = 0;

        opt = getopt_long(argc, argv, "f:z:t:", long_options, &option_index);
        if (opt == -1)
            break;

        switch (opt)
        {
        case 'f':
            file = optarg;
            break;
        case 'z':
            CHUNK_SIZE = std::atoi(optarg);
            break;
        case 't':
            num_threads = std::atoi(optarg);
            break;
        case '?':
            print_usage();
            return 1;
        default:
            break;
        }
    }
    

    if (CHUNK_SIZE <= 0 || num_threads <= 0){
        std::cerr << "Error: Missing required arguments!\n";
        print_usage();
        return 1;
    }

    size_t chunk_size_mb = CHUNK_SIZE * (1024 * 1024);
    std::cout << "Chunk size: " << chunk_size_mb << "\n";
    std::cout << "Number of Threads: " << num_threads << "\n";
    std::cout << "File: " << file << std::endl;

    auto start_time = std::chrono::high_resolution_clock::now();
    ThreadPool t_pool(num_threads);  

    int fd = open(file, O_RDONLY);
    if (fd == -1) {
        std::cerr << "Cannot open file: " << file << std::endl;
        return -1;
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        std::cerr << "Cannot get file size" << std::endl;
        close(fd);
        return -1;
    }

    char* data = static_cast<char*>(mmap(nullptr, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0));
    if (data == MAP_FAILED) {
        std::cerr << "Memory mapping failed" << std::endl;
        close(fd);
        return -1;
    }

    madvise(data, sb.st_size, MADV_SEQUENTIAL);

    std::vector<std::map<std::string, data_t>> thread_stores;
    std::mutex mergeTex; 

    size_t offset = 0;
    while (offset < (size_t)sb.st_size) {
        size_t chunk_end = std::min(offset + chunk_size_mb, (size_t)sb.st_size);
        
        while (chunk_end < (size_t)sb.st_size && data[chunk_end] != '\n') {
            chunk_end++;
        }

        madvise(data + offset, chunk_end - offset, MADV_WILLNEED);

        t_pool.addTask([=, &thread_stores, &mergeTex] {
            local_store.clear();
            process_chunk(data + offset, data + chunk_end);
            madvise(data + offset, chunk_end - offset, MADV_DONTNEED);

            std::lock_guard<std::mutex> lock(mergeTex);
            thread_stores.push_back(std::move(local_store));
        });

        offset = chunk_end + 1;
    }

    t_pool.stop();  

    std::map<std::string, data_t> store;
    for (const auto& local_map : thread_stores) {
        for (const auto& [key, data] : local_map) {
            auto& global_data = store[key];

            global_data.sum += data.sum;
            global_data.count += data.count;
            global_data.max = std::max(global_data.max, data.max);
            global_data.min = std::min(global_data.min, data.min);
        }
    }

    for (const auto& entry : store) {
        std::cout << std::fixed << std::setprecision(1) << entry.first << ": avg=" << entry.second.sum / entry.second.count << " min=" << entry.second.min << " max=" << entry.second.max << '\n';
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "Execution time: " << duration.count() << " milliseconds" << std::endl;

    munmap(data, sb.st_size);
    close(fd);

    return 0;
}
