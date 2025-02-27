#pragma once

#include <mutex>
#include <thread>
#include <condition_variable>
#include <functional>
#include <vector>

#include "queue.hpp"

class ThreadPool{
    public:
        ThreadPool(size_t num_threads = std::thread::hardware_concurrency()){
            // iniciar as threads e fazer com que estejam em loop a checkar se a task queue tem jobs
        };

        void addTask(std::function<void()> task);

        void collect();

        void stop();

    private:

        void workerThread();
        // sinal para distinguir entre esperar pelo trabalho ou parar.
        bool exit;
    
        std::vector<std::thread> threads;
        Queue<std::function<void()>> taskQueue;

        std::mutex poolTex;
        std::condition_variable cond;

        // é preciso notificar que o job está terminado para dar exit.
};