/*
 * ============================================================================
 * ESERCIZIO: Diverse Invocation Targets for std::thread Creation
 * ============================================================================
 * SCOPO:
 * Dimostrare le 5 diverse modalità di creazione di un `std::thread` in C++: 
 * 1. Funzione libera con parametri.
 * 2. Espressione Lambda.
 * 3. Metodo membro non-statico di una classe (tramite indirizzo dell'istanza).
 * 4. Metodo membro statico di una classe.
 * 5. Functor (Oggetto funzione che sovraccarica l'operator()).
 *
 * STEP:
 * 1. Definire le varie entità invocabili (`printRedMessageTask`, `LoggerTestService`, `PrintIterationFunctor`).
 * 2. Istanziare i singoli thread nel `main` per ciascuna delle 5 modalità target.
 * 3. Verificare la joinabilità dei thread tramite `.joinable()`.
 * 4. Sincronizzare la terminazione di tutti i thread eseguendo `.join()`.
 *
 * Tags: std::thread, callable-objects, lambda-expression, member-functions, functor, thread-instantiation
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <string>
#include <thread>

// Caso 1: Funzione libera
void printRedMessageTask(const std::string& messageText)
{
    LOG("\033[1;31m" << messageText << "\033[0m");
}

// Classe di test per Metodi Membro
class LoggerTestService
{
public:
    LoggerTestService() = default;

    // Caso 3: Metodo membro non-statico
    void printBlueMemberMessageTask() 
    {
        LOG("\033[1;34mHello World in Blue!\033[0m");
    }

    // Caso 4: Metodo membro statico
    static void printYellowStaticMessageTask()
    {
        LOG("\033[1;33mHello World in Yellow!\033[0m");
    }
};

// Caso 5: Functor (Oggetto Funzione)
struct PrintIterationFunctor 
{
    void operator()(int iterationCount) const
    {
        for (int iterationIndex = 0; iterationIndex < iterationCount; ++iterationIndex) 
        {
            LOG("Functor running: " << iterationIndex);
        }
    }
};

int main() 
{
    LOG("=== Thread Creation ===");
    LOG("Core logici disponibili: " << std::thread::hardware_concurrency());
    
    // Caso 1: Funzione libera con parametro
    const std::string redMessageText = "Hello World in Red!";
    std::thread workerThread1(printRedMessageTask, redMessageText);

    // Caso 2: Espressione Lambda con parametro
    std::thread workerThread2([](const std::string& messageText) {
        LOG("\033[1;32m" << messageText << "\033[0m");
    }, "Hello World in Green!");

    // Caso 3: Metodo membro di istanza (richiede il puntatore al metodo e l'indirizzo dell'oggetto)
    LoggerTestService testServiceInstance;
    std::thread workerThread3(&LoggerTestService::printBlueMemberMessageTask, &testServiceInstance);

    // Caso 4: Metodo membro statico (invocato come una funzione libera)
    std::thread workerThread4(&LoggerTestService::printYellowStaticMessageTask);

    // Caso 5: Functor con parametro
    std::thread workerThread5(PrintIterationFunctor{}, 3);

    // Gestione pulita e sicura del ciclo di vita dei thread
    if (workerThread1.joinable()) workerThread1.join();
    if (workerThread2.joinable()) workerThread2.join();
    if (workerThread3.joinable()) workerThread3.join();
    if (workerThread4.joinable()) workerThread4.join();
    if (workerThread5.joinable()) workerThread5.join();

    LOG("Tutti i thread hanno terminato con successo.");
    return 0;
}