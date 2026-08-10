/*
 * ============================================================================
 * ESERCIZIO: Lock-Free Queue using std::atomic
 * ============================================================================
 * SCOPO:
 * Implementare una coda concorrente lock-free basata sull'algoritmo di 
 * Michael & Scott (1996). Utilizza operazioni Compare-And-Swap 
 * (std::atomic::compare_exchange_weak) per garantire thread-safety, progresso 
 * concorrente e assenza di deadlock senza ricorrere ai mutex.
 *
 * STEP:
 * 1. Definire la struttura interna Node con il dato e un puntatore atomico al nodo successivo.
 * 2. Inizializzare la coda con un nodo fantoccio (dummy node) puntato sia da head_ che da tail_.
 * 3. Implementare il metodo push() mediante il pattern Help-Along per avanzare la coda in sicurezza.
 * 4. Implementare il metodo pop() verificando lo stato di coda vuota e liberando i vecchi nodi dummy.
 * 5. Eseguire un test di stress con molteplici produttori e consumatori concorrenti.
 *
 * Tags: std::atomic, std::atomic::compare_exchange_weak, lock-free, michael-scott, std::thread
 * ============================================================================
 */

#include "logger.h"
#include <cassert>
#include <iostream>
#include <vector>
#include <chrono>
#include <atomic>
#include <thread>

template <typename T>
class LockFreeQueue 
{
private:
    struct Node
    {
        T data_{};
        std::atomic<Node*> next_{nullptr};

        Node() = default;
        explicit Node(const T& value) : data_(value), next_{nullptr} {}
    };

    std::atomic<Node*> head_;
    std::atomic<Node*> tail_;

public:
    LockFreeQueue() 
    {
        Node* dummy = new Node();
        head_.store(dummy);
        tail_.store(dummy);
    }

    ~LockFreeQueue()
    {
        while (Node* oldHead = head_.load())
        {
            head_.store(oldHead->next_.load());
            delete oldHead;
        }
    }

    LockFreeQueue(const LockFreeQueue&) = delete;
    LockFreeQueue& operator=(const LockFreeQueue&) = delete;

    void push(const T& value)
    {
        Node* newNode = new Node(value);

        while (true)
        {
            Node* currentTail = tail_.load();
            Node* currentNext = currentTail->next_.load();

            if (currentTail == tail_.load())
            {
                if (currentNext == nullptr)
                {
                    if (currentTail->next_.compare_exchange_weak(currentNext, newNode)) 
                    {
                        tail_.compare_exchange_weak(currentTail, newNode);
                        return;
                    }
                }
                else
                {
                    // Help-Along Pattern: avanziamo la tail per conto di un altro thread
                    tail_.compare_exchange_weak(currentTail, currentNext);
                }
            }
        }
    }

    bool pop(T& result)
    {
        while (true)
        {
            Node* currentHead = head_.load();
            Node* currentTail = tail_.load();
            Node* currentNext = currentHead->next_.load();

            if (currentHead == head_.load())
            {
                if (currentHead == currentTail)
                {
                    if (currentNext == nullptr)
                    {
                        return false;
                    }
                    // La tail e' rimasta indietro durante un inserimento concorrente
                    tail_.compare_exchange_weak(currentTail, currentNext);
                }
                else
                {
                    result = currentNext->data_;

                    if (head_.compare_exchange_weak(currentHead, currentNext))
                    {
                        delete currentHead;
                        return true;
                    }
                }
            }
        }
    }
};

int main() 
{
    LOG("=== Test Lock-Free Queue (Michael & Scott Algorithm) ===");

    LockFreeQueue<int> queue;

    constexpr int numProducers = 4;
    constexpr int numConsumers = 4;
    constexpr int itemsPerThread = 1000;

    std::atomic<int> totalPushed{0};
    std::atomic<int> totalPopped{0};

    std::vector<std::thread> workerThreads;

    // Avvio dei thread Produttori
    for (int i = 0; i < numProducers; ++i)
    {
        workerThreads.emplace_back([&queue, &totalPushed, i]() {
            for (int j = 0; j < itemsPerThread; ++j)
            {
                int val = i * itemsPerThread + j;
                queue.push(val);
                totalPushed.fetch_add(1);
            }
        });
    }

    // Avvio dei thread Consumatori
    for (int i = 0; i < numConsumers; ++i)
    {
        workerThreads.emplace_back([&queue, &totalPopped]() {
            int poppedVal = 0;
            for (int j = 0; j < itemsPerThread; ++j)
            {
                while (!queue.pop(poppedVal)) 
                {
                    std::this_thread::yield();
                }
                totalPopped.fetch_add(1);
            }
        });
    }

    for (auto& t : workerThreads)
    {
        if (t.joinable())
        {
            t.join();
        }
    }

    LOG("Totale elementi inseriti dai Produttori: " << totalPushed.load());
    LOG("Totale elementi estratti dai Consumatori: " << totalPopped.load());

    assert(totalPushed.load() == totalPopped.load());
    LOG("=== TEST SUPERATO CON SUCCESSO (Lock-Free Safe) ===");

    return 0;
}