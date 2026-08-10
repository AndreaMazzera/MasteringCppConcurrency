/*
 * ============================================================================
 * ESERCIZIO: Concurrent Independent Even/Odd Printing
 * ============================================================================
 * SCOPO:
 * Dimostrare l'esecuzione concorrente e indipendente di due thread distinti 
 * per la stampa dei numeri pari e dispari fino a un limite definito dall'utente. 
 * Illustra la mancanza di sincronizzazione esplicita tra thread concorrenti.
 *
 * STEP:
 * 1. Definire la task `printEvenNumbersTask` per la stampa dei soli numeri pari.
 * 2. Definire la task `printOddNumbersTask` per la stampa dei soli numeri dispari.
 * 3. Acquisire il limite massimo dall'utente nel thread principale.
 * 4. Istanziare ed eseguire in parallelo i due thread coordinandone la chiusura tramite `.join()`.
 *
 * Tags: std::thread, std::thread::joinable, std::thread::join, concurrency, basic-multithreading
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <string>
#include <thread>

void printEvenNumbersTask(int maxLimit)
{
    int currentEvenNumber = 0;
    
    while (currentEvenNumber <= maxLimit)
    {
        LOG("[Pari] : " << currentEvenNumber);
        currentEvenNumber += 2;
    }
}

void printOddNumbersTask(int maxLimit)
{
    int currentOddNumber = 1;
    
    while (currentOddNumber <= maxLimit)
    {
        LOG("[Dispari]: " << currentOddNumber);
        currentOddNumber += 2;
    }
}

int main() 
{
    LOG("=== Stampa Indipendente Pari/Dispari ===");
    LOG("Core logici disponibili: " << std::thread::hardware_concurrency());
    
    int maxLimit = 0;
    LOG("Inserisci il valore massimo da non superare:");
    std::cin >> maxLimit;

    // Avvio dei thread indipendenti
    std::thread evenWorkerThread(printEvenNumbersTask, maxLimit);
    std::thread oddWorkerThread(printOddNumbersTask, maxLimit);

    // Attesa del completamento dei thread
    if (evenWorkerThread.joinable())
    {
        evenWorkerThread.join();
    }

    if (oddWorkerThread.joinable())
    {
        oddWorkerThread.join();
    }

    LOG("Esecuzione terminata. Tutti i numeri entro il limite sono stati stampati.");
    return 0;
}