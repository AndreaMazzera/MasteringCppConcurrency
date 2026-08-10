/*
 * ============================================================================
 * ESERCIZIO: Flexible Mutex Locking with std::unique_lock and Manual Unlock
 * ============================================================================
 * SCOPO:
 * Dimostrare l'utilizzo avanzato di `std::unique_lock` per la gestione flessibile 
 * dei mutex in C++. A differenza di `std::lock_guard`, `std::unique_lock` consente 
 * di sbloccare manualmente il mutex (`.unlock()`) prima della chiusura dello scope, 
 * minimizzando la durata della sezione critica per massimizzare il parallelismo effettivo.
 *
 * STEP:
 * 1. Definire la risorsa condivisa (`sharedProgressValue`) e il relativo mutex (`sharedProgressMutex`).
 * 2. In `executeHeavyTaskWork`, eseguire i calcoli intensivi locali senza acquisire il lock.
 * 3. Acquisire il mutex tramite `std::unique_lock` solo per il tempo strettamente necessario all'aggiornamento.
 * 4. Rilasciare esplicitamente il lock tramite `.unlock()` subito dopo la modifica della risorsa condivisa.
 * 5. Eseguire la fase di post-elaborazione e logging fuori dal mutex.
 * 6. Sincronizzare la terminazione dei thread nel `main` tramite `.join()` e verificare la correttezza del risultato.
 *
 * Tags: std-thread, std-unique-lock, std-mutex, critical-section, fine-grained-locking, concurrency, performance-optimization
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <mutex>
#include <chrono>
#include <vector>
#include <thread>

// Risorsa condivisa e relativo mutex di protezione
int sharedProgressValue = 0;
std::mutex sharedProgressMutex;

void executeHeavyTaskWork(int workerThreadId)
{
    constexpr int totalIterationSteps = 3;

    for (int stepIndex = 0; stepIndex < totalIterationSteps; ++stepIndex)
    {
        // 1. ELABORAZIONE INTENSIVA (Fuori dal Lock)
        // Simulazione di calcolo eseguito in vero parallelismo simultaneo da tutti i thread.
        LOG("[Thread " << workerThreadId << "] Performing heavy calculation step " << stepIndex << "...");
        std::this_thread::sleep_for(std::chrono::milliseconds(50)); 

        // 2. SEZIONE CRITICA (Sotto Lock)
        // Acquisizione immediata del mutex tramite std::unique_lock
        std::unique_lock<std::mutex> flexibleScopedLock(sharedProgressMutex);
        
        sharedProgressValue += 10;
        LOG("[Thread " << workerThreadId << "] -> CRITICAL: Updated shared progress to: " << sharedProgressValue);

        // 3. RILASCIO MANUALE DEL LOCK
        // Rilascio anticipato del mutex appena completata la modifica dello stato condiviso
        flexibleScopedLock.unlock();

        // 4. POST-ELABORAZIONE (Fuori dal Lock)
        // Operazioni successive eseguite in sicurezza senza mantenere impegnato il mutex
        LOG("[Thread " << workerThreadId << "] Logging local step completion.");
    }
}

int main() 
{
    LOG("=== Starting unique_lock Demo ===");

    constexpr int workerThreadsCount = 3;
    std::vector<std::thread> workerThreads;

    const auto executionStartTime = std::chrono::high_resolution_clock::now();

    // Avvio dei thread worker in concorrenza
    for (int threadIndex = 0; threadIndex < workerThreadsCount; ++threadIndex)
    {
        workerThreads.emplace_back(executeHeavyTaskWork, threadIndex);
    }

    // Attesa del completamento di tutti i thread
    for (auto& workerThread : workerThreads)
    {
        if (workerThread.joinable()) 
        {
            workerThread.join();
        }
    }

    const auto executionEndTime = std::chrono::high_resolution_clock::now();
    const auto totalExecutionDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
        executionEndTime - executionStartTime
    );

    LOG("=== Simulation Finished ===");
    LOG("Final Shared Progress: " << sharedProgressValue << " (Expected: " << workerThreadsCount * 3 * 10 << ")");
    LOG("Total execution time: " << totalExecutionDuration.count() << " ms");

    return 0;
}