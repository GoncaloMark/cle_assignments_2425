#include "threading.hpp"

void ThreadPool::addTask(std::function<void()> task){
    {
        std::unique_lock<std::mutex> lock(poolTex);
        taskQueue.enQueue(std::move(task));
    }
    cond.notify_one();
}

void ThreadPool::stop(){
    {
        std::unique_lock<std::mutex> lock(poolTex);
        exit = true;
    }

    cond.notify_all();

    for (auto& thread : threads) {
        thread.join();
    }
}