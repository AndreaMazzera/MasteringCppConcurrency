/*
 * ============================================================================
 * ESERCIZIO: Console Output Protection and Interleaving Prevention
 * ============================================================================
 * SCOPO:
 * Dimostrare l'effetto di una Race Condition sui flussi di output standard (`std::cout`) 
 * e la sua risoluzione mediante l'uso di `std::mutex` e `std::lock_guard`. 
 * Evidenzia come la mancata sincronizzazione generi un output mescolato (*interleaved*),
 * mentre l'uso del lock RAII garantisca l'atomicità logica dell'intera sequenza di stampa.
 *
 * STEP:
 * 1. Definire le costanti di sistema (`totalThreadsCount`) ed il mutex globale `consoleOutputMutex`.
 * 2. Implementare la task non protetta (`unprotectedConsolePrintTask`) per mostrare l'interleaving dei thread.
 * 3. Implementare la task protetta (`protectedConsolePrintTask`) che acquisisce `consoleOutputMutex` via `std::lock_guard`.
 * 4. Istanziare ed eseguire i pool di thread nel `main` per confrontare visivamente i due comportamenti.
 *
 * Tags: std::thread, std::mutex, std::lock_guard, std-cout-thread-safety, race-condition, output-interleaving
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <mutex>
#include <thread>

constexpr int totalThreadsCount = 10;
std::mutex consoleOutputMutex;

void unprotectedConsolePrintTask()
{
    // 1. Stampa dell'inizio del messaggio (ID Thread)
    std::cout << "[Thread " << std::this_thread::get_id() << "]"; 
    
    // Forzatura della sospensione dello scheduler per evidenziare l'interleaving
    std::this_thread::sleep_for(std::chrono::milliseconds(20)); 
    
    // 2. Stampa della parte finale del messaggio
    std::cout << " -> Operazione completata correttamente.\n";
}

void protectedConsolePrintTask()
{
    // Il lock RAII garantisce che l'intero blocco di stampa sia eseguito atomicamente da un solo thread
    std::lock_guard<std::mutex> consoleLock(consoleOutputMutex);
    
    std::cout << "[Thread " << std::this_thread::get_id() << "]"; 
    
    // La sospensione avviene mantenendo il lock, prevenendo l'interruzione da parte degli altri thread
    std::this_thread::sleep_for(std::chrono::milliseconds(20)); 
    
    std::cout << " -> Operazione completata correttamente.\n";
}

int main() 
{
    LOG("=== TEST 1: OUTPUT NON PROTETTO (RACE CONDITION) ===");
    std::vector<std::thread> unprotectedWorkerPool;
    unprotectedWorkerPool.reserve(totalThreadsCount);
    
    for (int threadIndex = 0; threadIndex < totalThreadsCount; ++threadIndex) 
    {
        unprotectedWorkerPool.emplace_back(unprotectedConsolePrintTask);
    }
    
    for (auto& workerThread : unprotectedWorkerPool) 
    {
        if (workerThread.joinable())
        {
            workerThread.join();
        }
    }

    LOG("\n=== TEST 2: OUTPUT PROTETTO DA MUTEX ===");
    std::vector<std::thread> protectedWorkerPool;
    protectedWorkerPool.reserve(totalThreadsCount);
    
    for (int threadIndex = 0; threadIndex < totalThreadsCount; ++threadIndex) 
    {
        protectedWorkerPool.emplace_back(protectedConsolePrintTask);
    }
    
    for (auto& workerThread : protectedWorkerPool) 
    {
        if (workerThread.joinable())
        {
            workerThread.join();
        }
    }

    LOG("Tutti i thread hanno terminato con successo.");
    return 0;
}