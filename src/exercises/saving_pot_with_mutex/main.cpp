/*
 * ============================================================================
 * ESERCIZIO: Shared Resource Protection with std::mutex and std::lock_guard
 * ============================================================================
 * SCOPO:
 * Dimostrare la protezione di una risorsa condivisa (`savingsPotBalance`) mediante
 * `std::mutex` per prevenire Race Condition. Utilizza il pattern RAII tramite 
 * `std::lock_guard` per garantire l'acquisizione e il rilascio sicuro del lock.
 *
 * STEP:
 * 1. Definire le risorse condivise (`savingsPotBalance` e `bankVaultMutex`).
 * 2. Implementare la task `depositMoneyTask` per incrementare il saldo in sezione critica.
 * 3. Implementare la task `withdrawMoneyTask` per decrementare il saldo in sezione critica.
 * 4. Istanziare i thread nel `main`, attenderne il completamento e verificare il saldo finale.
 *
 * Tags: std::thread, std::mutex, std::lock_guard, raii, race-condition, thread-safety
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <string>
#include <mutex>
#include <thread>

constexpr int totalTransactionsCount = 100000;

int savingsPotBalance = 0;
std::mutex bankVaultMutex;

void depositMoneyTask()
{
    for (int transactionIndex = 0; transactionIndex < totalTransactionsCount; ++transactionIndex)
    {
        std::lock_guard<std::mutex> lock(bankVaultMutex);
        savingsPotBalance += 1;
    }
}

void withdrawMoneyTask()
{
    for (int transactionIndex = 0; transactionIndex < totalTransactionsCount; ++transactionIndex)
    {
        std::lock_guard<std::mutex> lock(bankVaultMutex);
        savingsPotBalance -= 1;
    }
}

int main() 
{
    LOG("=== Shared Resource Protection ===");
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

    LOG("Saldo finale del salvadanaio: " << savingsPotBalance);
    LOG("Tutti i thread hanno terminato con successo.");
    
    return 0;
}