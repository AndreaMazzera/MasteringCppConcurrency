/*
 * ============================================================================
 * ESERCIZIO: Cooperative Thread Cancellation using std::atomic
 * ============================================================================
 * SCOPO:
 * Dimostrare il pattern di cancellazione cooperativa di un thread in background 
 * utilizzando un flag booleano atomico (`std::atomic<bool>`). Illustra come passare 
 * una variabile atomica allocata nello stack per riferimento costante (`std::cref`) 
 * a una task in loop, consentendo un'arresto pulito e thread-safe.
 *
 * STEP:
 * 1. Implementare la task `backgroundWorkerTask` che controlla atomicamente il flag via `.load()`.
 * 2. Istanziare la variabile atomica `stopSignalRequested` nel `main`.
 * 3. Avviare il thread worker `backgroundWorkerThread` passando il flag tramite `std::cref`.
 * 4. Inviare la richiesta di arresto con `.store(true)` dopo un intervallo di attesa e completare con `.join()`.
 *
 * Tags: std::thread, std::atomic, std-cref, thread-cancellation, cooperative-shutdown, thread-safety
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <string>
#include <atomic>
#include <chrono>
#include <thread>

void backgroundWorkerTask(const std::atomic<bool>& stopSignal, int taskId)
{
    LOG("[Task " << taskId << "] Avviato in background.");

    while (!stopSignal.load())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        LOG("[Task " << taskId << "] Sto lavorando...");   
    }

    LOG("[Task " << taskId << "] Segnale di stop ricevuto. Lavoro concluso.");
}

int main() 
{
    LOG("Core logici disponibili: " << std::thread::hardware_concurrency());
    
    // Il flag risiede nello stack del main(), evitando variabili globali
    std::atomic<bool> stopSignalRequested{false};

    // Passaggio della risorsa atomica tramite std::cref per evitare copie
    std::thread backgroundWorkerThread(backgroundWorkerTask, std::cref(stopSignalRequested), 1);

    // Il Main esegue le sue operazioni per 3 secondi
    std::this_thread::sleep_for(std::chrono::seconds(3));

    LOG("[Main] Invio richiesta di stop...");
    stopSignalRequested.store(true);

    if (backgroundWorkerThread.joinable())
    {
        backgroundWorkerThread.join();
    }

    LOG("[Main] Thread terminato correttamente. Fine programma.");
    return 0;
}