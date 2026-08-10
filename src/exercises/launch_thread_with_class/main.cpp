/*
 * ============================================================================
 * ESERCIZIO: Launch Thread with Class Member Function
 * ============================================================================
 * SCOPO:
 * Dimostrare l'incapsulamento dell'esecuzione di un thread all'interno di una 
 * classe C++, avviando una funzione membro privata tramite il puntatore `this` 
 * e consentendo la modifica in sicurezza dello stato dell'istanza.
 *
 * STEP:
 * 1. Definire la classe WorkerTask contenente un metodo privato di elaborazione e il proprio stato interno.
 * 2. Implementare un metodo pubblico `startAsync()` che istanzia e restituisce un `std::thread` legato all'istanza corrente (`this`).
 * 3. Avviare due istanze distinte di WorkerTask in parallelo dal thread principale.
 * 4. Attendere la terminazione di entrambi i thread tramite `.join()` e verificare la corretta mutazione dello stato negli oggetti.
 *
 * Tags: std::thread, std::thread::joinable, std::thread::join
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <string>
#include <chrono>
#include <thread>

class WorkerTask 
{
private:
    int workerId_;
    int itemsProcessed_{0};

    void executeWork(int totalToProcess, const std::string& taskName)
    {
        LOG("[Worker " << workerId_ << "] Inizio task '" << taskName << "'...");

        for (int i = 0; i < totalToProcess; ++i)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            ++itemsProcessed_;
            
            LOG("[Worker " << workerId_ << "] Processato elemento " 
                << itemsProcessed_ << "/" << totalToProcess);
        }

        LOG("[Worker " << workerId_ << "] Task '" << taskName << "' COMPLETATO.");
    }

public:
    explicit WorkerTask(int id) : workerId_(id) {}

    std::thread startAsync(int totalToProcess, const std::string& taskName)
    {
        // Il puntatore `this` consente al thread di accedere ai membri dell'istanza
        return std::thread(&WorkerTask::executeWork, this, totalToProcess, taskName);
    }

    int getItemsProcessed() const { return itemsProcessed_; }
};

int main()
{
    LOG("=== BENCHMARK: THREAD CON FUNZIONI MEMBRO ===");

    WorkerTask worker1(101);
    WorkerTask worker2(102);

    LOG("[Main] Avvio dei thread attraverso le istanze delle classi...");

    std::thread t1 = worker1.startAsync(3, "Download Dati");
    std::thread t2 = worker2.startAsync(5, "Elaborazione Immagini");

    if (t1.joinable()) t1.join();
    if (t2.joinable()) t2.join();

    LOG("--------------------------------------------------");
    LOG("[Main] Verifico lo stato finale aggiornato negli oggetti:");
    LOG("[Main] Worker 1 elementi processati: " << worker1.getItemsProcessed());
    LOG("[Main] Worker 2 elementi processati: " << worker2.getItemsProcessed());

    LOG("=== Fine del programma ===");
    return 0;
}