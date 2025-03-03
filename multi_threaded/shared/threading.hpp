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
                        // open a scope for unique_lock
                        {
                            // lock queue
                            std::unique_lock<std::mutex> lock(poolTex);
    
                            // wait for task to be put in queue
                            cond_work.wait(lock, [this] {
                                return !taskQueue.isEmpty() || exit;
                            });
    
                            // pool is stopped and there are no more tasks
                            if (exit && taskQueue.isEmpty()) {
                                return;
                            }
                            
                            task = std::move(taskQueue.deQueue());
                            busy++;

                            // Unlock to execute task
                            lock.unlock();

                            task();

                            // Lock to notify end of job
                            lock.lock();
                            busy--;
                            cond_finished.notify_one();
                        }
                    }
                });
            }
        };

        ~ThreadPool(){
            {
                std::unique_lock<std::mutex> lock(poolTex);
                exit = true;
            }
        
            cond_work.notify_all();
        
            for (auto& thread : threads) {
                thread.join();
            }
        }

        void addTask(std::function<void()> task);

        void waitFinished();

    private:
        // sinal para distinguir entre esperar pelo trabalho ou parar.
        bool exit = false;
    
        std::vector<std::thread> threads;
        Queue<std::function<void()>> taskQueue;

        mutable std::mutex poolTex;
        mutable std::condition_variable cond_work;

        unsigned int busy = 0;
        mutable std::condition_variable cond_finished;
};