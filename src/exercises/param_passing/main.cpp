/*
 * ============================================================================
 * ESERCIZIO: Thread Parameter Passing and Ownership Transfer Mechanics
 * ============================================================================
 * SCOPO:
 * Dimostrare la gestione avanzata dei parametri e del ciclo di vita di un `std::thread`:
 * 1. Lo scarto del valore di ritorno da parte del costruttore di `std::thread`.
 * 2. Il passaggio esplicito per riferimento tramite `std::ref`.
 * 3. Il trasferimento di risorse non copiabili (`std::unique_ptr`) tramite `std::move`.
 * 4. Il trasferimento della responsabilita di gestione (ownership) di un `std::thread`.
 *
 * STEP:
 * 1. Istanziare un thread con funzione avente return non-void per dimostrare la perdita del valore di ritorno.
 * 2. Invocare una funzione con parametro per riferimento modificabile avvolgendolo in `std::ref`.
 * 3. Trasferire un'istanza di `std::unique_ptr` nel contesto d'esecuzione di un thread tramite `std::move`.
 * 4. Creare un thread tramite factory function e trasferirne la proprieta tra variabili `std::thread`.
 *
 * Tags: std::thread, std::ref, std::move, std::unique_ptr, ownership-transfer, type-erasure
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <string>
#include <memory>
#include <utility>
#include <thread>

// 1. Il valore di ritorno non-void viene ignorato dal costruttore di std::thread
int calculateSquare(int number) 
{
    return number * number;
}

// 2. Passaggio per riferimento modificabile
void incrementValue(int& valueToIncrement) 
{
    valueToIncrement += 1;
}

// 3. Passaggio di una risorsa move-only (std::unique_ptr)
void processResource(std::unique_ptr<int> resourcePtr) 
{
    if (resourcePtr) 
    {
        LOG("Risorsa elaborata nel thread. Valore: " << *resourcePtr);
    }
}

// 4. Factory function per il trasferimento di ownership del thread
std::thread createWorkerThread(int workerId) 
{
    return std::thread([workerId]() {
        LOG("Worker temporaneo #" << workerId << " in esecuzione.");
    });
}

int main() 
{
    LOG("=== Typing and Parameters Passing ===");

    // Caso A: Valore di ritorno scartato
    std::thread returnThread(calculateSquare, 5);
    returnThread.join();
    LOG("Thread con return terminato (il valore e andato perduto).");

    // Caso B: Passaggio per riferimento obbligatorio tramite std::ref
    int value = 10;
    std::thread refThread(incrementValue, std::ref(value)); 
    refThread.join();
    LOG("Valore di x dopo il thread (atteso 11): " << value);

    // Caso C: Spostamento di oggetti non copiabili (std::unique_ptr)
    auto resourcePtr = std::make_unique<int>(42);
    std::thread moveThread(processResource, std::move(resourcePtr));
    moveThread.join();

    // Caso D: Trasferimento di ownership di un oggetto std::thread
    std::thread receivedThread = createWorkerThread(1);
    
    // Spostamento esplicito tra istanze di std::thread
    std::thread destinationThread = std::move(receivedThread);
    
    LOG("receivedThread.joinable(): " << (receivedThread.joinable() ? "true" : "false"));
    LOG("destinationThread.joinable(): " << (destinationThread.joinable() ? "true" : "false"));
    
    destinationThread.join();

    return 0;
}