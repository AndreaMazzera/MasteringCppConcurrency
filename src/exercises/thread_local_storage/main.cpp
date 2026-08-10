/*
 * ============================================================================
 * ESERCIZIO: Thread Local Storage (TLS) with thread_local
 * ============================================================================
 * SCOPO:
 * Dimostrare l'uso della specificazione `thread_local` per istanziare variabili 
 * il cui stato e durata sono legati al ciclo di vita del singolo thread. 
 * Evidenzia l'isolamento totale della memoria tra thread diversi (incluso il `main`), 
 * eliminando la contesa sulla memoria e la necessita di sincronizzazione tramite mutex.
 *
 * STEP:
 * 1. Definire la variabile globale `thread_local` `globalThreadSpecificCounter`.
 * 2. Implementare la task `processTaskWorker` con uno stato statico locale ed un generatore PRNG thread-local.
 * 3. Assegnare un valore al counter nello stack del `main` e lanciare un pool di worker thread.
 * 4. Attendere i thread tramite `.join()` e verificare che il valore del `main` rimanga inalterato.
 *
 * Tags: std::thread, thread_local, thread-local-storage, memory-isolation, lock-free, random-number-generation
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <chrono>
#include <thread>

// Variabile globale Thread-Local (ogni thread possiede una copia separata)
thread_local int globalThreadSpecificCounter = 0;

void processTaskWorker(int threadId)
{
    // Variabile statica Thread-Local: mantiene lo stato per chiamata, MA solo per lo STESSO thread
    thread_local static int functionExecutionCount = 0;
    ++functionExecutionCount;

    // Generatore di numeri casuali isolato per thread (evita la contesa su std::mt19937)
    thread_local std::mt19937 randomEngine(std::random_device{}());
    std::uniform_int_distribution<int> uniformDistribution(10, 50);

    LOG("[Thread " << threadId << "] Avvio esecuzione. Indirizzo di globalThreadSpecificCounter: " 
        << &globalThreadSpecificCounter);

    for (int iterationIndex = 0; iterationIndex < 3; ++iterationIndex)
    {
        // Incremento TLS sicuro senza necessita di mutex
        globalThreadSpecificCounter += 10;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(uniformDistribution(randomEngine)));

        LOG("[Thread " << threadId << "] Ciclo " << iterationIndex + 1 
            << " | Counter Locale = " << globalThreadSpecificCounter 
            << " | Chiamate Funzione = " << functionExecutionCount);
    }

    LOG("[Thread " << threadId << "] COMPLETATO. Valore finale Counter Locale = " 
        << globalThreadSpecificCounter);
}

int main()
{
    LOG("=== BENCHMARK: THREAD LOCAL STORAGE (thread_local) ===");

    constexpr int totalWorkerThreadsCount = 3;
    std::vector<std::thread> workerThreads;
    workerThreads.reserve(totalWorkerThreadsCount);

    LOG("[Main] Modifico globalThreadSpecificCounter nel thread principale...");
    globalThreadSpecificCounter = 999;
    LOG("[Main] Valore nel Main Thread: " << globalThreadSpecificCounter 
        << " (Indirizzo: " << &globalThreadSpecificCounter << ")");
    LOG("--------------------------------------------------");

    // Avvio dei thread worker
    for (int threadIndex = 1; threadIndex <= totalWorkerThreadsCount; ++threadIndex)
    {
        workerThreads.emplace_back(processTaskWorker, threadIndex);
    }

    // Attesa completamento di tutti i thread
    for (auto& workerThread : workerThreads)
    {
        if (workerThread.joinable())
        {
            workerThread.join();
        }
    }

    LOG("--------------------------------------------------");
    // Verifica di isolamento: il valore nel thread principale e rimasto inalterato
    LOG("[Main] Controllo finale nel Main Thread: " << globalThreadSpecificCounter 
        << " (E rimasto 999, totalmente isolato!)");

    LOG("=== Fine del programma ===");
    return 0;
}