/*
 * ============================================================================
 * ESERCIZIO: Basic Multithreading and Thread Array Management
 * ============================================================================
 * SCOPO:
 * Dimostrare la creazione, il tracciamento e la sincronizzazione di un array 
 * fisso di thread worker (`std::thread`). Illustra il passaggio di parametri 
 * per valore alle task dei thread e l'attesa sequenziale del loro completamento 
 * tramite `.join()`.
 *
 * STEP:
 * 1. Definire la task `printCounterTask` per stampare una sequenza di conteggi numerati.
 * 2. Istanziare un array di `std::thread` nel `main`.
 * 3. Inizializzare ed avviare ciascun thread nel ciclo di creazione passandogli il limite di iterazioni.
 * 4. Iterare sull'array per verificare la joinabilità con `.joinable()` ed attenderne la chiusura con `.join()`.
 *
 * Tags: std::thread, multithreading-basics, thread-array, thread-management, joinable
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <string>
#include <thread>

constexpr int totalWorkerThreadsCount = 4;

void printCounterTask(int targetIterationCount)
{
    for (int iterationIndex = 0; iterationIndex < targetIterationCount; ++iterationIndex)
    {
        LOG("\033[1;31m[Thread " << targetIterationCount << "]: Conteggio " << iterationIndex + 1 << "\033[0m");
    }
}

int main() 
{
    LOG("=== Thread Creation ===");
    LOG("Core logici disponibili: " << std::thread::hardware_concurrency());
    
    std::thread workerThreads[totalWorkerThreadsCount];

    for (int threadIndex = 0; threadIndex < totalWorkerThreadsCount; ++threadIndex)
    {
        const int iterationLimit = threadIndex + 1;
        workerThreads[threadIndex] = std::thread(printCounterTask, iterationLimit);
    }

    for (int threadIndex = 0; threadIndex < totalWorkerThreadsCount; ++threadIndex)
    {
        if (workerThreads[threadIndex].joinable()) 
        {
            workerThreads[threadIndex].join();
        }
    }

    LOG("Tutti i thread hanno terminato con successo.");
    return 0;
}