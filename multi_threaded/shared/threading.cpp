#include "threading.hpp"

void ThreadPool::addTask(std::function<void()> task){
    // dar lock e pôr lá na queue o job
}

void ThreadPool::collect(){
    // chamar stop, fazer join, enviar os resultados.
}

void ThreadPool::stop(){
    // dar lock, mudar flag exit.
}