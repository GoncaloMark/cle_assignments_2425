#include <iomanip>
#include <fstream>
#include <map>
#include <chrono>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "threading.hpp"
#include "../include/datastructures.hpp"

constexpr size_t CHUNK_SIZE = 2 * (1024 * 1024); // 8MB chunk size

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

int main(int argc, char* argv[]) {
    const char* file = "measurements.txt";
    if (argc > 1) {
        file = argv[1];
    }

    auto start_time = std::chrono::high_resolution_clock::now();
    ThreadPool t_pool(4);  

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
        size_t chunk_end = std::min(offset + CHUNK_SIZE, (size_t)sb.st_size);
        
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
