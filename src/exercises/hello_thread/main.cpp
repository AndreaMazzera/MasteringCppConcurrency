/*
 * ============================================================================
 * ESERCIZIO: Hello World Thread (Thread Creation & Join)
 * ============================================================================
 * SCOPO:
 * Dimostrare la creazione base di un thread in C++ tramite std::thread, 
 * l'esecuzione di un task delegato e la sincronizzazione con il thread 
 * principale mediante .join().
 *
 * STEP:
 * 1. Definire la funzione libera print() da eseguire nel thread secondario.
 * 2. Rilevare e stampare il numero di core logici disponibili sulla macchina.
 * 3. Istanziare un oggetto std::thread passandogli la funzione da eseguire.
 * 4. Verificare tramite .joinable() che il thread sia valido e sincronizzare la terminazione con .join().
 *
 * Tags: std::thread, std::thread::hardware_concurrency, std::thread::joinable, std::thread::join
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <string>
#include <thread>

void print()
{
    LOG("\033[1;32mHello World By a Thread!\033[0m");
}

int main() 
{
    LOG("Core logici disponibili: " << std::thread::hardware_concurrency());
    
    std::thread threadWorker(print);
    
    if (threadWorker.joinable())
    {
        threadWorker.join();
    }

    return 0;
}