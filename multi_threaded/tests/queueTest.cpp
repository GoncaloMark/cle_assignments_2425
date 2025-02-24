#include <cassert>

#include "queue.hpp"

void testQueue() {
    Queue<int> q;

    assert(q.isEmpty() && "Queue should be empty initially");

    q.enQueue(10);
    q.enQueue(20);
    q.enQueue(30);

    assert(!q.isEmpty() && "Queue should not be empty after enQueue");

    assert(q.deQueue() == 10 && "First deQueue should return 10");
    assert(q.deQueue() == 20 && "Second deQueue should return 20");
    assert(q.deQueue() == 30 && "Third deQueue should return 30");

    assert(q.isEmpty() && "Queue should be empty after all dequeues");

    q.enQueue(10);
    q.enQueue(20);
    q.enQueue(30);

    q.clear();

    assert(q.isEmpty() && "Queue should be empty after clearing");
}

int main() {
    testQueue();
    return 0;
}
