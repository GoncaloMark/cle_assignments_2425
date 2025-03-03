#include <iomanip>
#include <fstream>
#include <map>
#include <chrono>
#include <cstring>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h> 

#include "threading.hpp"
#include "datastructures.hpp"

static option long_options[] = {
    {"file", required_argument, 0, 'f'},
    {"chunk_size", required_argument, 0, 'z'},
    {"threads", required_argument, 0, 't'},
    {0,         0,           0,  0 }
};

void print_usage() {
    std::cerr << "Usage: ./cle-ws-mt -f <file> -z <chunk_size> -t <threads>\n"
              << "  -f, --file <file>         Input file (required)\n"
              << "  -z, --chunk_size <size>  Chunk size in MB (required)\n"
              << "  -t, --threads <num>       Number of threads (required)\n";
}

int main(int argc, char* argv[]) {
    int opt;
    size_t chunk_size = -1;
    int num_threads = -1;
    const char* file = "measurements.txt";

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
            chunk_size = std::atoi(optarg);
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
    

    if (chunk_size <= 0 || num_threads <= 0){
        std::cerr << "Error: Missing required arguments!\n";
        print_usage();
        return 1;
    }

    size_t chunk_size_b = chunk_size * (1024 * 1024);
    std::cout << "Chunk size: " << chunk_size_b << "\n";
    std::cout << "Number of Threads: " << num_threads << "\n";
    std::cout << "File: " << file << "\n";

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

    std::map<std::string, data_t> store;
    std::mutex storeTex; 

    size_t offset = 0;
    while (offset < (size_t)sb.st_size) {
        size_t chunk_end = std::min(offset + chunk_size_b, (size_t)sb.st_size);
        
        while (chunk_end < (size_t)sb.st_size && data[chunk_end] != '\n') {
            chunk_end++;
        }

        madvise(data + offset, chunk_end - offset, MADV_WILLNEED);

        t_pool.addTask([=, &store, &storeTex] {
            const char* ptr = data + offset;
            while (ptr < (data + chunk_end)) {
                const char* line_end = static_cast<const char*>(std::memchr(ptr, '\n', (data + chunk_end) - ptr));
                if (!line_end) break;

                const char* semicolon = static_cast<const char*>(std::memchr(ptr, ';', line_end - ptr));
                if (!semicolon) {
                    std::cerr << "Skipping malformed line (missing ';'): " << std::string(ptr, line_end - ptr) << '\n';
                } else {
                    try {
                        std::string key(ptr, semicolon - ptr);
                        float value = std::stof(std::string(semicolon + 1, line_end - (semicolon + 1)));

                        {
                            std::lock_guard<std::mutex> lock(storeTex);
                            auto& data = store[key];
                            data.sum += value;
                            data.count += 1;
                            data.max = std::max(data.max, value);
                            data.min = std::min(data.min, value);
                        }
                    } catch (const std::exception& e) {
                        std::cerr << "Skipping invalid line: " << std::string(ptr, line_end - ptr) << '\n';
                    }
                }
                ptr = line_end + 1;  // go next line
            }
            madvise(data + offset, chunk_end - offset, MADV_DONTNEED);
        });

        offset = chunk_end + 1;
    }

    t_pool.waitFinished();  

    for (const auto& entry : store) {
        std::cout << std::fixed << std::setprecision(1) << entry.first << ": avg=" << entry.second.sum / entry.second.count << " min=" << entry.second.min << " max=" << entry.second.max << '\n';
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "Execution time: " << duration.count() << " milliseconds" << std::endl;

    t_pool.addTask([=, &store, &storeTex] {
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        std::cout << "after job" << std::endl;
    });

    t_pool.waitFinished();  

    munmap(data, sb.st_size);
    close(fd);

    return 0;
}
