#include "threading.hpp"

void ThreadPool::addTask(std::function<void()> task){
    {
        std::unique_lock<std::mutex> lock(poolTex);
        taskQueue.enQueue(std::move(task));
    }
    cond_work.notify_one();
}

void ThreadPool::waitFinished(){
    std::unique_lock<std::mutex> lock(poolTex);
    cond_finished.wait(lock, [this](){ return taskQueue.isEmpty() && (busy == 0); });
}