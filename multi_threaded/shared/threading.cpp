#include "threading.hpp"

void ThreadPool::addTask(std::function<void()> task){
    {
    // dar lock e pôr lá na queue o job
    std::lock_guard<std::mutex> lock(poolTex);
    taskQueue.enQueue(task);
    }
    cond.notify_one(); //"Acorda" uma thread para executar a tarefa
}

void ThreadPool::collect(){
    // chamar stop, fazer join, enviar os resultados.
    stop();
}

void ThreadPool::stop(){
    // dar lock, mudar flag exit.
    {
        std::lock_guard<std::mutex> lock(poolTex);
        exit = true;
    }
    cond.notify_all();

    for (std::thread &t : threads){
        if (t.joinable()){
            t.join();
        }
    }
}

void ThreadPool::workerThread(){
    while (true){}
    {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock(poolTex);
            cond.wait(lock, [this] {
                return exit || !taskQueue.isEmpty();
            });

            if (exit && taskQueue.isEmpty()){
                return;
            }

            task = taskQueue.deQueue();
        }

        task();
    }
    
}