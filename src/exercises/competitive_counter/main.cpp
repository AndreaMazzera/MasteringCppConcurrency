/*
 * ============================================================================
 * ESERCIZIO: Competitive Counter (Contatore Concorrente e Lock Contention)
 * ============================================================================
 * SCOPO:
 * Confrontare le prestazioni e la correttezza di tre approcci alla modifica 
 * di una risorsa condivisa: non protetto (Data Race), con un lock per ogni 
 * iterazione (Heavy Contention), e con accumulo locale e un unico lock finale (Optimized).
 *
 * STEP:
 * 1. Fase Unprotected: Incrementare un contatore globale senza sincronizzazione per evidenziare la Data Race.
 * 2. Fase Heavy Lock: Proteggere ogni singolo incremento con un std::lock_guard all'interno del ciclo.
 * 3. Fase Optimized Lock: Utilizzare una variabile locale per gli incrementi e acquisire il lock una sola volta al termine per unire i risultati.
 * 4. Misurare e stampare i tempi di esecuzione delle tre strategie.
 *
 * Tags: std::thread, std::mutex, std::lock_guard
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>

int globalCounter = 0;
std::mutex counterMutex;

int main() 
{
    LOG("Available Hardware Cores: " << std::thread::hardware_concurrency());
    
    std::vector<std::thread> threads(4);

    // =================================================================================================
    // 1. Unprotected - Data Race Demonstration
    // =================================================================================================
    auto startUnprotected = std::chrono::high_resolution_clock::now();

    for(int i = 0; i < 4; ++i)
    {
        threads[i] = std::thread([]() {
            for (int j = 0; j < 100000; ++j)
                ++globalCounter;
        });
    }

    for(auto& t : threads)
    {
        if (t.joinable()) t.join();
    }

    auto endUnprotected = std::chrono::high_resolution_clock::now();
    auto durationUnprotected = std::chrono::duration_cast<std::chrono::milliseconds>(endUnprotected - startUnprotected);

    LOG("Unprotected Result (Expect random < 400000): " << globalCounter);
    LOG("Unprotected Time: " << durationUnprotected.count() << " ms");
    
    globalCounter = 0;

    // =================================================================================================
    // 2. Heavy Lock Overhead (Lock ad ogni iterazione)
    // =================================================================================================
    auto startHeavyLock = std::chrono::high_resolution_clock::now();

    for(int i = 0; i < 4; ++i)
    {
        threads[i] = std::thread([]() {
            for (int j = 0; j < 100000; ++j)
            {
                std::lock_guard<std::mutex> lock(counterMutex);
                ++globalCounter;
            }
        });
    }

    for(auto& t : threads)
    {
        if (t.joinable()) t.join();
    }

    auto endHeavyLock = std::chrono::high_resolution_clock::now();
    auto durationHeavyLock = std::chrono::duration_cast<std::chrono::milliseconds>(endHeavyLock - startHeavyLock);

    LOG("Heavy Lock Guard Result (Expect exact 400000): " << globalCounter);
    LOG("Heavy Lock Guard Time: " << durationHeavyLock.count() << " ms");

    globalCounter = 0;

    // =================================================================================================
    // 3. Single Lock per thread (Accumulo locale + Singolo Lock finale)
    // =================================================================================================
    auto startOptimizedLock = std::chrono::high_resolution_clock::now();

    for(int i = 0; i < 4; ++i)
    {
        threads[i] = std::thread([]() {
            int localCounter = 0;
            for (int j = 0; j < 100000; ++j)
            {
                ++localCounter;
            }
            
            std::lock_guard<std::mutex> lock(counterMutex);
            globalCounter += localCounter;
        });
    }

    for(auto& t : threads)
    {
        if (t.joinable()) t.join();
    }

    auto endOptimizedLock = std::chrono::high_resolution_clock::now();
    auto durationOptimizedLock = std::chrono::duration_cast<std::chrono::milliseconds>(endOptimizedLock - startOptimizedLock);

    LOG("Optimized Lock Result (Expect exact 400000): " << globalCounter);
    LOG("Optimized Lock Time: " << durationOptimizedLock.count() << " ms");

    return 0;
}