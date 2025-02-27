#pragma once

#include <mutex>
#include <thread>
#include <condition_variable>
#include <functional>
#include <vector>
#include <iostream>

#include "queue.hpp"

class ThreadPool{
    public:
        ThreadPool(size_t num_threads = std::thread::hardware_concurrency()){
            for (size_t i = 0; i < num_threads; ++i) {
                threads.emplace_back([this] {
                    while (true) {
                        std::function<void()> task;
                        // open a scope so queue is unlocked before executing the task
                        {
                            // lock queue
                            std::unique_lock<std::mutex> lock(poolTex);
    
                            // wait for task to be put in queue
                            cond.wait(lock, [this] {
                                return !taskQueue.isEmpty() || exit;
                            });
    
                            // pool is stopped and there are no more tasks
                            if (exit && taskQueue.isEmpty()) {
                                return;
                            }
                            
                            task = std::move(taskQueue.deQueue());
                        }
                        
                        task();
                    }
                });
            }
        };

        void addTask(std::function<void()> task);

        void stop();

    private:
        // sinal para distinguir entre esperar pelo trabalho ou parar.
        bool exit = false;
    
        std::vector<std::thread> threads;
        Queue<std::function<void()>> taskQueue;

        mutable std::mutex poolTex;
        mutable std::condition_variable cond;

        // é preciso notificar que o job está terminado para dar exit.
};