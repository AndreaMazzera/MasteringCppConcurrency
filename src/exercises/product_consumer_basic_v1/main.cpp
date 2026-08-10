/*
 * ============================================================================
 * ESERCIZIO: Classic Producer-Consumer Pattern with Condition Variable
 * ============================================================================
 * SCOPO:
 * Dimostrare l'implementazione del pattern Producer-Consumer Single-Producer 
 * Single-Consumer (SPSC) utilizzando una coda condivisa `std::queue`, `std::mutex` 
 * e `std::condition_variable`. Illustra l'esecuzione delle operazioni di I/O 
 * e simulazione di carico al di fuori della sezione critica per minimizzare 
 * il tempo di ritenzione del lock.
 *
 * STEP:
 * 1. Definire le risorse condivise (`sharedQueue`, `syncMutex`, `conditionVar` e la flag `isProductionFinished`).
 * 2. Implementare la task `producerWorkerTask`: genera dati fuori dal lock, acquisisce il mutex per la `push` e notifica il consumatore.
 * 3. Implementare la task `consumerWorkerTask`: attende notifiche via `wait`, preleva un elemento, rilascia il lock ed elabora il dato.
 * 4. Istanziare i thread nel `main` e coordinarne il completamento pulito tramite `.join()`.
 *
 * Tags: std::thread, std::mutex, std::condition_variable, std::queue, producer-consumer, thread-synchronization
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <string>
#include <queue>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <thread>

std::mutex syncMutex;
std::condition_variable conditionVar;
std::queue<int> sharedQueue;
bool isProductionFinished = false;

void producerWorkerTask()
{
    for (int i = 0; i < 5; ++i)
    {
        // Simulazione della generazione del dato al di fuori del lock
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); 

        {
            std::lock_guard<std::mutex> lock(syncMutex);
            LOG("Produttore: Aggiungo elemento " << i);
            sharedQueue.push(i);
        }
        
        conditionVar.notify_one();
    }

    // Segnalazione del completamento della produzione
    {
        std::lock_guard<std::mutex> lock(syncMutex);
        isProductionFinished = true;
    }
    conditionVar.notify_all();
}

void consumerWorkerTask()
{
    while (true)
    {
        std::unique_lock<std::mutex> lock(syncMutex);
        
        // Attesa finche sono presenti elementi in coda OPPURE la produzione e terminata
        conditionVar.wait(lock, [] { return !sharedQueue.empty() || isProductionFinished; });

        if (sharedQueue.empty() && isProductionFinished) 
        {
            break;
        }

        // Prelievo dell'elemento dalla coda
        const int consumedValue = sharedQueue.front();
        sharedQueue.pop();
        LOG("Consumatore: Tolgo elemento -> " << consumedValue);

        lock.unlock(); // Rilascio del lock prima dell'elaborazione intensiva
        
        // Simulazione dell'elaborazione del dato al di fuori del lock
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }
}

int main() 
{
    LOG("Core logici disponibili: " << std::thread::hardware_concurrency());
    
    std::thread producerThread(producerWorkerTask);
    std::thread consumerThread(consumerWorkerTask);

    if (producerThread.joinable())
    {
        producerThread.join();
    }

    if (consumerThread.joinable())
    {
        consumerThread.join();
    }

    LOG("Esecuzione terminata pulita. Elementi rimasti in coda: " << sharedQueue.size());

    return 0;
}