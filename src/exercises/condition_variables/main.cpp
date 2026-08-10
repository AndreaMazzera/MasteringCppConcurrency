/*
 * ============================================================================
 * ESERCIZIO: Condition Variables
 * ============================================================================
 * SCOPO:
 * Sincronizzare l'esecuzione di due thread utilizzando una Condition Variable,
 * permettendo a un thread lavoratore di rimanere in attesa efficiente finché 
 * un flag di stato non viene impostato a true dal thread principale.
 *
 * STEP:
 * 1. Avviare un thread lavoratore che acquisisce un std::unique_lock e si mette in attesa tramite cv.wait().
 * 2. Simulare del lavoro nel thread principale tramite un intervallo di sleep.
 * 3. Modificare lo stato del flag (ready = true) sotto la protezione di std::lock_guard.
 * 4. Notificare il thread in attesa tramite cv.notify_one() e attendere il suo completamento con .join().
 *
 * Tags: std::thread, std::mutex, std::unique_lock, std::lock_guard, std::condition_variable
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>

std::mutex mtx;
std::condition_variable cv;
bool ready = false;

void print()
{
    std::unique_lock<std::mutex> lock(mtx);
    
    // Il predicato lambda evita risvegli spuri (spurious wakeups)
    cv.wait(lock, []{ return ready; });
    
    LOG("\033[1;32mHello World By a Thread!\033[0m");
}

int main() 
{
    LOG("Core logici disponibili: " << std::thread::hardware_concurrency());
    
    std::thread threadWorker(print);

    std::this_thread::sleep_for(std::chrono::milliseconds(2000)); 

    {
        std::lock_guard<std::mutex> lock(mtx);
        ready = true;
    }
    cv.notify_one();

    if (threadWorker.joinable())
        threadWorker.join();

    LOG("Thread terminato con successo!");

    return 0;
}