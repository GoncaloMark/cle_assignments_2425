#pragma once 

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
        }

        void enQueue(const T element);
        T deQueue();
        void clear();

    private:
        Node* first;
        Node* last;
};

template <typename T>
void Queue<T>::enQueue(const T element){
    if(isEmpty()){
        first = new Node(element);
        last = first;
    } else {
        Node *p = new Node(element);
        last->next = p;
        last = last->next;
    }
}

template <typename T>
T Queue<T>::deQueue(){
    T element;
    Node *p;
    if(!isEmpty()){
        element = first->element;

        p = first;
        first = first->next;

        delete p;
    }
    return element;
}

template <typename T>
void Queue<T>::clear(){
    while(!isEmpty())
        deQueue();
}