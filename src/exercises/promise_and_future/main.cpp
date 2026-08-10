/*
 * ============================================================================
 * ESERCIZIO: Inter-Thread Communication using std::promise and std::future
 * ============================================================================
 * SCOPO:
 * Dimostrare l'uso del canale di comunicazione unirezionale `std::promise` e 
 * `std::future` per trasferire un risultato calcolato in modo asincrono da un 
 * worker thread al thread principale (`main`). Evidenzia la necessita di muovere 
 * (`std::move`) il `std::promise` essendo un tipo move-only.
 *
 * STEP:
 * 1. Creare un oggetto `std::promise<int>` ed estrarne il relativo `std::future<int>`.
 * 2. Avviare la task `performHeavyCalculationTask` trasferendo la proprieta esclusiva del promise via `std::move`.
 * 3. Eseguire elaborazioni concorrenti sul thread principale durante il calcolo.
 * 4. Bloccare la ricezione sul `main` tramite `resultFuture.get()` ed attendere il completamento del thread worker tramite `.join()`.
 *
 * Tags: std::thread, std::promise, std::future, asynchronous-programming, move-semantics, thread-synchronization
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <string>
#include <chrono>
#include <future>
#include <thread>

void performHeavyCalculationTask(std::promise<int> resultPromise)
{
    LOG("[Worker] Avviato. Inizio dell'elaborazione dei dati segreti...");
    
    // Simulazione di 2 secondi di calcolo inteso
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    constexpr int computedResult = 999;
    
    LOG("[Worker] Calcolo terminato. Spedisco il risultato nel canale...");
    
    // Inserimento del valore nel promise per sbloccare il futuro
    resultPromise.set_value(computedResult);
    
    // Operazioni di pulizia successive alla trasmissione
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    LOG("[Worker] Pulizia completata ed uscita dal thread.");
}

int main()
{
    LOG("=== BENCHMARK: STD::PROMISE & STD::FUTURE ===");

    // 1. Creazione del canale di comunicazione (lato di input)
    std::promise<int> resultPromise;
    
    // 2. Estrazione del lato di output associato al promise
    std::future<int> resultFuture = resultPromise.get_future();

    // 3. Lancio del worker thread trasferendo il promise in modo esclusivo
    LOG("[Main] Lancio del thread Worker fisicamente...");
    std::thread workerThread(performHeavyCalculationTask, std::move(resultPromise));

    // 4. Operazioni concorrenti eseguite dal thread principale
    LOG("[Main] Sto eseguendo altre operazioni concorrenti mentre l'hardware lavora...");
    for (int stepIndex = 0; stepIndex < 3; ++stepIndex) 
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
        LOG("[Main] Aggiornamento interfaccia grafica (" << stepIndex + 1 << "/3)...");
    }

    // 5. Riscossione del dato dal futuro
    LOG("[Main] In attesa del risultato dal canale (.get())...");
    
    const int retrievedResult = resultFuture.get(); 

    LOG("[Main] Segnale RICEVUTO! Il valore calcolato e: " << retrievedResult);

    // Chiusura pulita del thread worker
    if (workerThread.joinable()) 
    {
        workerThread.join();
    }

    LOG("=== Fine del programma ===");
    return 0;
}