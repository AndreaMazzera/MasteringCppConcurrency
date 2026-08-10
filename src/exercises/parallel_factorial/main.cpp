/*
 * ============================================================================
 * ESERCIZIO: Thread Creation and Reference Passing
 * ============================================================================
 * SCOPO:
 * Dimostrare il passaggio di argomenti per riferimento a una funzione eseguita 
 * su un thread secondario tramite std::ref. Illustra come recuperare il 
 * risultato di un calcolo senza l'uso di std::future o valori di ritorno.
 *
 * STEP:
 * 1. Definire la funzione `computeFactorial` che accetta un valore in ingresso e un riferimento di output.
 * 2. Acquisire l'input dall'utente tramite std::cin.
 * 3. Istanziare std::thread passando la funzione, il parametro per valore e il riferimento avvolto da std::ref.
 * 4. Sincronizzare il thread tramite .join() e stampare il risultato aggiornato nel main thread.
 *
 * Tags: std::thread, std::ref, std::thread::joinable, std::thread::join, pass-by-reference
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <string>
#include <functional>
#include <thread>

void computeFactorial(int number, int& resultOutput)
{
    LOG("Avviato calcolo del fattoriale per: " << number);

    resultOutput = 1; 
    for (int i = number; i > 1; --i)
    {
        resultOutput *= i;
    }
    
    LOG("Calcolo completato nel thread secondario.");
}

int main() 
{
    LOG("=== Thread Creation - Esercizio 1B ===");
    LOG("Core logici disponibili: " << std::thread::hardware_concurrency());
   
    int inputNumber = 0;
    int factorialResult = 0;
    
    LOG("Inserisci un numero di cui calcolare il fattoriale:");
    std::cin >> inputNumber;

    std::thread workerThread(computeFactorial, inputNumber, std::ref(factorialResult));
    
    if (workerThread.joinable()) 
    {
        workerThread.join();
    }

    LOG("Risultato finale stampato dal main: " << factorialResult);
    LOG("Tutti i thread hanno terminato con successo.");

    return 0;
}