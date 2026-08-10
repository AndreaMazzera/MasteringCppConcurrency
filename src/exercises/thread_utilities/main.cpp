/*
 * ============================================================================
 * ESERCIZIO: Standard Thread Utilities (std::this_thread & Hardware Queries)
 * ============================================================================
 * SCOPO:
 * Dimostrare l'utilizzo delle utility offerte dal namespace `std::this_thread` 
 * per la gestione e il controllo dell'esecuzione del thread corrente, unitamente 
 * all'ispezione delle risorse hardware disponibili tramite `std::thread::hardware_concurrency()`.
 *
 * STEP:
 * 1. Rilevare il numero di core logici dell'hardware con `std::thread::hardware_concurrency()`.
 * 2. Inizializzare un thread lavoratore (`workerThread`) eseguendo una funzione lambda.
 * 3. Recuperare l'identificativo univoco del thread tramite `std::this_thread::get_id()`.
 * 4. Mettere in pausa il thread per una durata relativa con `std::this_thread::sleep_for()`.
 * 5. Sospendere il thread fino ad un time point assoluto specifico tramite `std::this_thread::sleep_until()`.
 * 6. Cedere volontariamente la quantizzazione di CPU allo scheduler di sistema via `std::this_thread::yield()`.
 * 7. Sincronizzare l'esecuzione nel `main` con `.join()`.
 *
 * Tags: std-thread, std-this-thread, std-chrono, hardware-concurrency, sleep-for, sleep-until, yield, thread-id
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <chrono>
#include <thread>

int main() 
{
    LOG("=== Thread Utility Functions ===");

    // 1. Interrogazione dell'hardware per determinare la capacita di parallelismo ed evitare oversubscription
    const unsigned int availableHardwareCores = std::thread::hardware_concurrency();
    LOG("Core logici rilevati dal sistema: " << availableHardwareCores);

    // 2. Avvio di un thread dedicato per testare le utility di std::this_thread
    std::thread workerThread([]() {
        // Recupero dell'identificativo univoco assegnato dall'OS al thread corrente
        LOG("ID identificativo assegnato dall'OS: " << std::this_thread::get_id());

        // Sospensione relativa tramite sleep_for
        constexpr auto duration150ms = std::chrono::milliseconds(150);
        LOG("Thread in sospensione per " << duration150ms.count() << " millisecondi...");
        std::this_thread::sleep_for(duration150ms);

        // Sospensione assoluta fino ad un time point futuro tramite sleep_until
        constexpr auto duration50ms = std::chrono::milliseconds(50);
        const auto futureTimePoint = std::chrono::steady_clock::now() + duration50ms;
        LOG("Thread in attesa fino a un time point specifico...");
        std::this_thread::sleep_until(futureTimePoint);

        // Rilascio volontario del timeslice allo scheduler del sistema operativo
        LOG("Rilascio volontario del core allo scheduler (yield)...");
        std::this_thread::yield();

        LOG("Thread worker completato.");
    });

    // Sincronizzazione della terminazione del thread
    if (workerThread.joinable()) 
    {
        workerThread.join();
    }

    LOG("=== Fine esecuzione ===");
    return 0;
}