/*
 * ============================================================================
 * ESERCIZIO: Atomic vs Non-Atomic Increments
 * ============================================================================
 * SCOPO:
 * Confrontare gli effetti dell'accesso concorrente non protetto a una variabile 
 * intera standard (Data Race) rispetto all'utilizzo di un tipo atomico 
 * (`std::atomic_int`), evidenziando la correttezza delle operazioni atomiche 
 * senza l'ausilio di mutex.
 *
 * STEP:
 * 1. Definire una variabile globale non atomica e una atomica (`std::atomic_int`).
 * 2. Creare una funzione che incrementa entrambe le variabili per 100.000 volte in un ciclo.
 * 3. Avviare 5 thread concorrenti e memorizzarli in un vettore tramite `std::move`.
 * 4. Sincronizzare tutti i thread con `.join()` e stampare i risultati finali per evidenziare la perdita di aggiornamenti nella variabile non atomica.
 *
 * Tags: std::thread, std::atomic, std::atomic_int
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>

int counter = 0;
std::atomic_int counterAtomic{0};

void increment()
{
    for (int i = 0; i < 100000; ++i)
    {
        ++counter;
        ++counterAtomic;
    }
}

int main() 
{
    LOG("Core logici disponibili: " << std::thread::hardware_concurrency());
    
    std::vector<std::thread> threads;
    threads.reserve(5);

    for (int i = 0; i < 5; ++i)
    {
        threads.emplace_back(increment);
    }

    for (auto& thread : threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }

    LOG("Variabile non atomica (valore errato per Data Race): " << counter);
    LOG("Variabile atomica (valore corretto atteso 500000): " << counterAtomic);

    return 0;
}