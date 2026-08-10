/*
 * ============================================================================
 * ESERCIZIO: Ping-Pong Thread Synchronization using Condition Variables
 * ============================================================================
 * SCOPO:
 * Dimostrare l'alternanza coordinata dell'esecuzione tra due thread distinti 
 * ("Ping" e "Pong") mediante l'uso combinato di `std::mutex`, `std::unique_lock` 
 * e `std::condition_variable` per evitare spin-lock e garantire la sincronizzazione.
 *
 * STEP:
 * 1. Definire le risorse di sincronizzazione condivise (mutex, flag booleano e condition variable).
 * 2. Implementare la task `pingWorkerTask` che attende il flag `isPingTurn == false`, stampa "Ping", aggiorna lo stato e notifica il thread Pong.
 * 3. Implementare la task `pongWorkerTask` che attende il flag `isPingTurn == true`, stampa "Pong", ripristina lo stato e notifica il thread Ping.
 * 4. Istanziare i due thread nel `main`, attendere il completamento di entrambi tramite `.join()` e verificare la corretta sequenza di messaggi.
 *
 * Tags: std::thread, std::mutex, std::unique_lock, std::condition_variable, thread-synchronization, ping-pong
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <string>
#include <mutex>
#include <condition_variable>
#include <thread>

std::mutex syncMutex;
std::condition_variable cvCondition;
bool isPingTurn = false;

void pingWorkerTask()
{
    for (int i = 0; i < 10; ++i)
    {
        std::unique_lock<std::mutex> lock(syncMutex);
        cvCondition.wait(lock, [] { return !isPingTurn; });

        LOG("Ping");
        isPingTurn = true;
        cvCondition.notify_one();
    }
}

void pongWorkerTask()
{
    for (int i = 0; i < 10; ++i)
    {
        std::unique_lock<std::mutex> lock(syncMutex);
        cvCondition.wait(lock, [] { return isPingTurn; });

        LOG("Pong");
        isPingTurn = false;
        cvCondition.notify_one();
    }
}

int main() 
{
    LOG("=== Ping-Pong Thread Synchronization ===");
    LOG("Core logici disponibili: " << std::thread::hardware_concurrency());
    
    std::thread pingThread(pingWorkerTask);
    std::thread pongThread(pongWorkerTask);

    if (pingThread.joinable()) 
    {
        pingThread.join();
    }

    if (pongThread.joinable()) 
    {
        pongThread.join();
    }

    LOG("Tutti i thread hanno terminato con successo.");
    return 0;
}