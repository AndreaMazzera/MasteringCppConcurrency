/*
 * ============================================================================
 * ESERCIZIO: Benchmark Mutex vs std::atomic Performance
 * ============================================================================
 * SCOPO:
 * Confrontare le prestazioni in termini di tempo di esecuzione tra la 
 * sincronizzazione basata su mutex (`std::lock_guard`) e quella basata su 
 * primitive atomiche hardware (`std::atomic`). Viene applicato l'allineamento 
 * a 64-byte (`alignas`) per prevenire fenomeni di False Sharing nella cache.
 *
 * STEP:
 * 1. Definire le funzioni di incremento per il contatore protetto da mutex e per quello atomico.
 * 2. Avviare 8 thread concorrenti per eseguire gli incrementi con mutex e misurarne la durata con std::chrono.
 * 3. Ripetere lo stesso benchmark utilizzando il contatore atomico `std::atomic<long long>`.
 * 4. Stampare a schermo i risultati dei contatori e il confronto prestazionale in millisecondi.
 *
 * Tags: std::thread, std::mutex, std::lock_guard, std::atomic, alignas, false-sharing, std::chrono
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <atomic>
#include <mutex>

constexpr int numThreads = 8;
constexpr int incrementsPerThread = 100000;

std::mutex counterMutex;

alignas(64) long long mutexCounter = 0;
alignas(64) std::atomic<long long> atomicCounter{0};

void incrementWithMutex()
{
    for (int i = 0; i < incrementsPerThread; ++i)
    {
        std::lock_guard<std::mutex> lock(counterMutex);
        ++mutexCounter;
    }
}

void incrementWithAtomic()
{
    for (int i = 0; i < incrementsPerThread; ++i)
    {
        ++atomicCounter;
    }
}

int main() 
{
    LOG("Core logici disponibili: " << std::thread::hardware_concurrency());
    
    // --- BENCHMARK 1: MUTEX ---
    std::vector<std::thread> workerThreads;
    workerThreads.reserve(numThreads);

    const auto startMutexTime = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < numThreads; ++i)
    {
        workerThreads.emplace_back(incrementWithMutex);
    }

    for (auto& threadWorker : workerThreads)
    {
        if (threadWorker.joinable())
        {
            threadWorker.join();
        }
    }

    const auto endMutexTime = std::chrono::high_resolution_clock::now();
    const auto mutexDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endMutexTime - startMutexTime);

    // --- BENCHMARK 2: ATOMIC ---
    workerThreads.clear();

    const auto startAtomicTime = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < numThreads; ++i)
    {
        workerThreads.emplace_back(incrementWithAtomic);
    }

    for (auto& threadWorker : workerThreads)
    {
        if (threadWorker.joinable())
        {
            threadWorker.join();
        }
    }

    const auto endAtomicTime = std::chrono::high_resolution_clock::now();
    const auto atomicDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endAtomicTime - startAtomicTime);

    LOG("--------------------------------------------------");
    LOG("Mutex Counter:  " << mutexCounter);
    LOG("Mutex Time:     " << mutexDuration.count() << " ms");
    LOG("--------------------------------------------------");
    LOG("Atomic Counter: " << atomicCounter.load());
    LOG("Atomic Time:    " << atomicDuration.count() << " ms");

    return 0;
}