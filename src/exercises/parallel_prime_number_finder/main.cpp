/*
 * ============================================================================
 * ESERCIZIO: Dynamic Chunking Load Balancing for Prime Search
 * ============================================================================
 * SCOPO:
 * Dimostrare la tecnica di bilanciamento del carico dinamico (Dynamic Chunking) 
 * applicata al calcolo intensivo non omogeneo (ricerca di numeri primi). 
 * Sfrutta `std::atomic<int>::fetch_add` con `std::memory_order_relaxed` per 
 * la prenotazione ultra-veloce dei blocchi di lavoro, riducendo la contesa dei 
 * lock e massimizzando l'occupazione dei core.
 *
 * STEP:
 * 1. Definire la funzione di verifica di primalita `isPrime` con complessita O(sqrt(N)).
 * 2. Implementare la worker task `dynamicChunkingWorker` che acquisisce blocchi di lavoro dinamici via atomic fetch_add.
 * 3. Raccogliere i risultati parziali all'interno di un vettore locale per evitare contese di sincronizzazione.
 * 4. Unire i risultati locali nel vettore globale con una singola acquisizione di `std::mutex` per thread.
 * 5. Avviare un pool di worker thread, attendere il completamento con `.join()` e misurare il tempo di esecuzione totale.
 *
 * Tags: std::thread, std::atomic, std::memory_order_relaxed, std::mutex, std::lock_guard, load-balancing, dynamic-chunking
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
#include <mutex>
#include <atomic>
#include <thread>

// Algoritmo di primalita O(sqrt(N)) ottimizzato
bool isPrime(int number) 
{
    if (number <= 1) return false;
    if (number <= 3) return true;
    if (number % 2 == 0 || number % 3 == 0) return false;
    
    for (int i = 5; i * i <= number; i += 6) 
    {
        if (number % i == 0 || number % (i + 2) == 0) return false;
    }
    return true;
}

// Funzione Worker con Dynamic Chunking
void dynamicChunkingWorker(
    std::atomic<int>& currentIndex, 
    int maxLimit, 
    int chunkSize, 
    std::vector<int>& globalPrimes, 
    std::mutex& resultMutex)
{
    std::vector<int> localPrimes;
    localPrimes.reserve(chunkSize / 5); // Stima preliminare per contenere le riallocazioni

    while (true)
    {
        // 1. Prenotazione atomica ultra-veloce del blocco
        const int start = currentIndex.fetch_add(chunkSize, std::memory_order_relaxed);
        
        if (start >= maxLimit) 
        {
            break; // Carico di lavoro esaurito
        }

        const int end = std::min(start + chunkSize, maxLimit);

        // 2. Calcolo intensivo isolato SENZA I/O e SENZA Lock
        for (int i = start; i < end; ++i)
        {
            if (isPrime(i))
            {
                localPrimes.push_back(i);
            }
        }
    }

    // 3. Unione dei risultati locali nel vettore globale (lock brevissimo eseguito 1 volta per thread)
    std::lock_guard<std::mutex> lock(resultMutex);
    globalPrimes.insert(globalPrimes.end(), localPrimes.begin(), localPrimes.end());
}

int main() 
{
    constexpr int maxLimit = 500000;
    constexpr int chunkSize = 2500; // Chunk Size ideale per il bilanciamento dinamico
    const unsigned int numThreads = std::thread::hardware_concurrency();

    LOG("Core logici utilizzati: " << numThreads);
    LOG("Ricerca primi da 1 a " << maxLimit << " con Dynamic Chunking (Size: " << chunkSize << ")...");

    std::atomic<int> currentIndex{1};
    std::vector<int> globalPrimes;
    std::mutex resultMutex;

    std::vector<std::thread> workerThreads;
    workerThreads.reserve(numThreads);

    const auto startTime = std::chrono::high_resolution_clock::now();

    // Avvio dei thread
    for (unsigned int i = 0; i < numThreads; ++i)
    {
        workerThreads.emplace_back(
            dynamicChunkingWorker, 
            std::ref(currentIndex), 
            maxLimit, 
            chunkSize, 
            std::ref(globalPrimes), 
            std::ref(resultMutex)
        );
    }

    // Attesa completamento dei worker thread
    for (auto& threadWorker : workerThreads)
    {
        if (threadWorker.joinable())
        {
            threadWorker.join();
        }
    }

    const auto endTime = std::chrono::high_resolution_clock::now();
    const auto totalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    LOG("=== ELABORAZIONE COMPLETATA IN " << totalDuration << " ms ===");
    LOG("Totale numeri primi trovati: " << globalPrimes.size());

    return 0;
}