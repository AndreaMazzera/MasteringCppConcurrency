/*
 * ============================================================================
 * ESERCIZIO: Data Race Demonstration on Shared Resource
 * ============================================================================
 * SCOPO:
 * Dimostrare il fenomeno della Data Race ed Undefined Behavior mediante 
 * l'accesso concorrente e non sincronizzato a una variabile globale condivisa 
 * (`savingsPotBalance`) da parte di due thread distinti.
 *
 * STEP:
 * 1. Definire la risorsa condivisa non protetta `savingsPotBalance`.
 * 2. Implementare la task `depositMoneyTask` per incrementare la risorsa senza lock.
 * 3. Implementare la task `withdrawMoneyTask` per decrementare la risorsa senza lock.
 * 4. Istanziare ed eseguire i thread nel `main`, attendere il completamento con `.join()` e mostrare l'esito inconsistente.
 *
 * Tags: std::thread, data-race, race-condition, undefined-behavior, unsynchronized-access
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <string>
#include <thread>

constexpr int totalTransactionsCount = 100000;

int savingsPotBalance = 0;

void depositMoneyTask()
{
    for (int transactionIndex = 0; transactionIndex < totalTransactionsCount; ++transactionIndex)
    {
        // Data Race: Accesso concorrente in scrittura senza sincronizzazione
        savingsPotBalance += 1;
    }
}

void withdrawMoneyTask()
{
    for (int transactionIndex = 0; transactionIndex < totalTransactionsCount; ++transactionIndex)
    {
        // Data Race: Accesso concorrente in scrittura senza sincronizzazione
        savingsPotBalance -= 1;
    }
}

int main() 
{
    LOG("=== Data Race Demonstration ===");
    LOG("Core logici disponibili: " << std::thread::hardware_concurrency());
    
    std::thread depositThread(depositMoneyTask);
    std::thread withdrawThread(withdrawMoneyTask);

    if (depositThread.joinable()) 
    {
        depositThread.join();
    }

    if (withdrawThread.joinable()) 
    {
        withdrawThread.join();
    }

    LOG("Saldo finale del salvadanaio (inconsistente): " << savingsPotBalance);
    LOG("Tutti i thread hanno terminato con successo.");
    
    return 0;
}