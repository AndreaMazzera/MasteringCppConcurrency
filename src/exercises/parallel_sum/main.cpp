/*
 * ============================================================================
 * ESERCIZIO: Vector Accumulation - Single vs Multi-Threaded Benchmark
 * ============================================================================
 * SCOPO:
 * Confrontare le prestazioni tra il calcolo della somma dei dati in modalità 
 * sequenziale e parallela (tramite decomposizione in intervalli contigui) 
 * su un ampio vettore di elementi. Illustra la fase di riduzione parziale 
 * (Reduction Phase) e misura lo Speedup ottenuto.
 *
 * STEP:
 * 1. Allocare un vettore di 100 milioni di elementi inizializzati a 1.
 * 2. Eseguire la somma sequenziale su un singolo thread e calcolarne la durata con std::chrono.
 * 3. Partizionare il vettore in intervalli distinti basati sul numero di thread disponibili.
 * 4. Avviare i worker thread che calcolano le somme parziali in modo indipendente senza contese di lock.
 * 5. Sincronizzare i thread via .join(), sommare i risultati parziali nel main thread e calcolare lo Speedup.
 *
 * Tags: std::thread, std::cref, std::ref, std::chrono, reduction-phase, benchmark, speedup
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <thread>

void calculatePartialSum(const std::vector<long long>& numbersVector, long long& partialResultOutput, int startIdx, int endIdx)
{
    long long localSum = 0; 
    for (int i = startIdx; i <= endIdx; ++i)
    {
        localSum += numbersVector[i];
    }
    partialResultOutput = localSum;
}

int main() 
{
    constexpr size_t totalElements = 100000000;
    std::vector<long long> numbersVector(totalElements, 1);
    long long totalResult = 0;

    // --- EXECUTION 1: SINGLE-THREADED ---
    const auto startSingleTime = std::chrono::high_resolution_clock::now();
    
    calculatePartialSum(numbersVector, totalResult, 0, static_cast<int>(numbersVector.size() - 1));

    const auto endSingleTime = std::chrono::high_resolution_clock::now();
    const auto singleThreadDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endSingleTime - startSingleTime);

    LOG("Single-Thread Result: " << totalResult);
    LOG("Single-Thread Computation Time: " << singleThreadDuration.count() << " ms");

    // Ripristino del risultato per la prova in multi-threading
    totalResult = 0; 

    // --- EXECUTION 2: MULTI-THREADED ---
    const int hardwareCores = std::thread::hardware_concurrency();
    const int usedThreads = std::max(1, hardwareCores / 2);
    const int elementsPerThread = static_cast<int>(numbersVector.size()) / usedThreads;

    LOG("Threads spawned: " << usedThreads);

    std::vector<std::thread> workerThreads;
    workerThreads.reserve(usedThreads);
    std::vector<long long> partialResults(usedThreads, 0);

    const auto startMultiTime = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < usedThreads; ++i)
    {
        const int startIdx = i * elementsPerThread;        
        const int endIdx = (i == usedThreads - 1) ? static_cast<int>(numbersVector.size() - 1) : (startIdx + elementsPerThread - 1);
        
        workerThreads.emplace_back(calculatePartialSum, std::cref(numbersVector), std::ref(partialResults[i]), startIdx, endIdx);
    }

    // Attesa completamento di tutti i worker thread
    for (auto& threadWorker : workerThreads)
    {
        if (threadWorker.joinable())
        {
            threadWorker.join();
        }
    }

    // Fase di riduzione (Reduction Phase)
    for (int i = 0; i < usedThreads; ++i)
    {
        totalResult += partialResults[i];
    }

    const auto endMultiTime = std::chrono::high_resolution_clock::now();
    const auto multiThreadDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endMultiTime - startMultiTime);

    LOG("Multi-Thread Result: " << totalResult);
    LOG("Multi-Thread Computation Time: " << multiThreadDuration.count() << " ms");

    // Calcolo dello Speedup
    const double speedup = static_cast<double>(singleThreadDuration.count()) / multiThreadDuration.count();
    LOG("Performance Speedup: " << speedup << "x");

    return 0;
}