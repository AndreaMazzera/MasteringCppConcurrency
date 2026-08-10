/*
 * ============================================================================
 * ESERCIZIO: Async Tasks & Futures Aggregation
 * ============================================================================
 * SCOPO:
 * Dimostrare il pattern Fork-Join per il calcolo parallelo asincrono mediante 
 * std::async e std::future. Viene illustrata la gestione di un vettore di 
 * future move-only, la riscossione bloccante dei risultati via .get() e 
 * l'aggregazione dei dati calcolati in parallelo.
 *
 * STEP:
 * 1. Definire la funzione di supporto `downloadAndProcessTask` che simula un'operazione I/O-bound o CPU-bound con latenza.
 * 2. Avviare un pool di task asincroni tramite std::async specificando la politica std::launch::async.
 * 3. Conservare i riferimenti std::future all'interno di un std::vector pre-allocato.
 * 4. Raccogliere e aggregare i risultati con std::accumulate, misurando il tempo totale di esecuzione.
 *
 * Tags: std::async, std::future, std::launch::async, std::accumulate, fork-join, concurrency
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <string>
#include <vector>
#include <numeric>
#include <chrono>
#include <future>
#include <thread>

int downloadAndProcessTask(int taskId)
{
    int sleepDuration = ((taskId % 3) + 1) * 500; 

    LOG("[Task " << taskId << "] Avviato. Richiedera' " << sleepDuration << "ms...");
    std::this_thread::sleep_for(std::chrono::milliseconds(sleepDuration));

    int result = taskId * 10;
    LOG("[Task " << taskId << "] Completato! Risultato calcolato: " << result);
    return result;
}

int main()
{
    LOG("=== BENCHMARK: TASKS ASINCRONI MULTIPLI ===");
    constexpr int totalTasks = 6;

    std::vector<std::future<int>> taskFutures;
    taskFutures.reserve(totalTasks);

    const auto startTime = std::chrono::high_resolution_clock::now();

    // 1. FASE DI FORK: Lancio simultaneo dei task asincroni
    LOG("[Main] Lancio di " << totalTasks << " task in background...");
    for (int i = 0; i < totalTasks; ++i)
    {
        taskFutures.emplace_back(std::async(std::launch::async, downloadAndProcessTask, i + 1));
    }

    LOG("[Main] Tutti i task sono in esecuzione parallela.");
    LOG("[Main] Raccolta dei risultati in corso (fase di Join)...");

    // 2. FASE DI JOIN: Riscossione dei risultati
    std::vector<int> taskResults;
    taskResults.reserve(totalTasks);

    int currentTaskIndex = 1;
    for (auto& taskFuture : taskFutures)
    {
        int value = taskFuture.get(); 
        taskResults.push_back(value);
        LOG("[Main] Ricevuto risultato dal Task " << currentTaskIndex++ << " -> Valore: " << value);
    }

    const int totalSum = std::accumulate(taskResults.begin(), taskResults.end(), 0);

    const auto endTime = std::chrono::high_resolution_clock::now();
    const auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    LOG("--------------------------------------------------");
    LOG("[Main] Elaborazione completata con successo!");
    LOG("[Main] Somma totale dei risultati: " << totalSum);
    LOG("[Main] Tempo totale impiegato: " << elapsedTime.count() << "ms");

    return 0;
}