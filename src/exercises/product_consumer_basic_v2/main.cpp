/*
 * ============================================================================
 * ESERCIZIO: Bounded Producer-Consumer Pattern with Graceful Shutdown
 * ============================================================================
 * SCOPO:
 * Dimostrare l'implementazione del pattern Producer-Consumer con buffer a 
 * capacita limitata (Bounded Queue), due `std::condition_variable` per gestire 
 * la coda piena e vuota, e un meccanismo di arresto sicuro (graceful shutdown) 
 * basato su `std::atomic<bool>`.
 *
 * STEP:
 * 1. Definire le risorse condivise (`sharedBoundedQueue`, `syncMutex`, `queueNotEmptyCondition`, `queueNotFullCondition`, `maxQueueCapacity`).
 * 2. Implementare la task `producerWorkerTask`: attende che la coda abbia spazio, inserisce elementi e gestisce lo spegnimento se `isProductionActive` passa a `false`.
 * 3. Implementare la task `consumerWorkerTask`: sblocca il produttore via `queueNotFullCondition` ed elabora gli elementi residui fino a svuotamento.
 * 4. Nel `main`, simulare un'esecuzione temporizzata, disattivare il produttore e attendere la terminazione pulita con `.join()`.
 *
 * Tags: std::thread, std::mutex, std::condition_variable, std::atomic, bounded-queue, producer-consumer, graceful-shutdown
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <string>
#include <queue>
#include <atomic>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <thread>

constexpr std::size_t maxQueueCapacity = 3;

std::mutex syncMutex;
std::condition_variable queueNotEmptyCondition;
std::condition_variable queueNotFullCondition;
std::queue<int> sharedBoundedQueue;

bool isProductionFinished = false;
std::atomic<bool> isProductionActive{true};

void producerWorkerTask()
{
    int generatedSequenceId = 0;

    while (isProductionActive)
    {
        // Simulazione della generazione del dato al di fuori del lock
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        std::unique_lock<std::mutex> lock(syncMutex);
        
        // Attesa finche la coda e piena O viene richiesto l'arresto della produzione
        queueNotFullCondition.wait(lock, [] {
            return sharedBoundedQueue.size() < maxQueueCapacity || !isProductionActive;
        });

        if (!isProductionActive)
        {
            break;
        }

        ++generatedSequenceId;
        LOG("Produttore: Aggiungo elemento " << generatedSequenceId);
        sharedBoundedQueue.push(generatedSequenceId);

        queueNotEmptyCondition.notify_one();
    }

    // Segnalazione del completamento della produzione
    {
        std::lock_guard<std::mutex> lock(syncMutex);
        isProductionFinished = true;
    }
    
    queueNotEmptyCondition.notify_all();
    LOG("Produttore: Spento.");
}

void consumerWorkerTask()
{
    while (true)
    {
        std::unique_lock<std::mutex> lock(syncMutex);

        // Attesa finche sono presenti elementi in coda OPPURE la produzione e terminata
        queueNotEmptyCondition.wait(lock, [] {
            return !sharedBoundedQueue.empty() || isProductionFinished;
        });

        if (sharedBoundedQueue.empty() && isProductionFinished)
        {
            break;
        }

        // Prelievo dell'elemento dalla coda
        const int consumedValue = sharedBoundedQueue.front();
        sharedBoundedQueue.pop();
        LOG("Consumatore: Tolgo elemento -> " << consumedValue);

        queueNotFullCondition.notify_one();

        lock.unlock(); // Rilascio del lock prima dell'elaborazione intensiva

        // Simulazione dell'elaborazione del dato al di fuori del lock
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }

    LOG("Consumatore: Coda vuota e produzione finita. Spento.");
}

int main()
{
    LOG("Core logici disponibili: " << std::thread::hardware_concurrency());

    std::thread producerThread(producerWorkerTask);
    std::thread consumerThread(consumerWorkerTask);

    // Il programma lavora attivamente per 3 secondi
    std::this_thread::sleep_for(std::chrono::seconds(3));

    LOG("[Main] Tempo scaduto! Spengo il sistema...");
    
    // Disattivazione sicura del ciclo del produttore
    isProductionActive = false;
    queueNotFullCondition.notify_all(); // Sblocca il produttore se bloccato su coda piena

    if (producerThread.joinable())
    {
        producerThread.join();
    }

    if (consumerThread.joinable())
    {
        consumerThread.join();
    }

    LOG("Esecuzione terminata pulita. Elementi rimasti in coda: " << sharedBoundedQueue.size());

    return 0;
}