#pragma once 
#include <stdexcept>


template <typename T>
class Queue{
    class Node{
        public:
            Node(T element = 0) : element(element), next(nullptr){};
            T element;
            Node* next;
    };

    public:
        Queue() : first(nullptr), last(nullptr){};

        inline bool isEmpty() const {
            return first == nullptr;
        };

        void enQueue(T&& element);
        T deQueue();
        void clear();

    private:
        Node* first;
        Node* last;
};

template <typename T>
void Queue<T>::enQueue(T&& element) {  
    if (isEmpty()) {
        first = new Node(std::move(element));  
        last = first;
    } else {
        Node* p = new Node(std::move(element));  
        last->next = p;
        last = last->next;
    }
}

template <typename T>
T Queue<T>::deQueue() {
    if (isEmpty()) {
        throw std::runtime_error("Queue is empty!"); 
    }

    Node *p = first;
    T element = std::move(p->element);  

    first = first->next;
    delete p;

    if (first == nullptr) {
        last = nullptr;  
    }

    return element; 
}

template <typename T>
void Queue<T>::clear(){
    while(!isEmpty())
        deQueue();
}