/*
 * ============================================================================
 * ESERCIZIO: Thread Safety con RAII (ThreadGuard Pattern)
 * ============================================================================
 * SCOPO:
 * Dimostrare la gestione sicura del ciclo di vita di un std::thread mediante 
 * il pattern RAII (ThreadGuard), prevenendo chiamate involontarie a 
 * std::terminate() durante uno stack unwinding causato da eccezioni, e 
 * illustrare il comportamento dell'indicatore .joinable().
 *
 * STEP:
 * 1. Definire la classe ThreadGuard che incapsula un riferimento a std::thread ed effettua il .join() nel distruttore.
 * 2. Monitorare i cambi di stato della proprietà .joinable() nelle fasi di creazione, esecuzione e terminazione.
 * 3. Mostrare il rischio di dangling reference associato all'utilizzo improprio di .detach().
 * 4. Simulare un'eccezione a runtime verificando la corretta pulizia del thread via RAII.
 *
 * Tags: std::thread, std::thread::joinable, std::thread::join, std::thread::detach, RAII, Exception Safety
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <string>
#include <chrono>
#include <stdexcept>
#include <thread>

class ThreadGuard 
{
private:
    std::thread& threadRef_;

public:
    explicit ThreadGuard(std::thread& t) : threadRef_(t) {}

    ~ThreadGuard() 
    {
        if (threadRef_.joinable()) 
        {
            LOG("-> [RAII] ThreadGuard interviene nel distruttore: eseguo join automatico.");
            threadRef_.join();
        }
    }

    ThreadGuard(const ThreadGuard&) = delete;
    ThreadGuard& operator=(const ThreadGuard&) = delete;
};

void processLongTask() 
{
    LOG("Sto eseguendo un compito lungo...");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void executeRiskyOperation() 
{
    std::thread worker(processLongTask);
    ThreadGuard guard(worker);

    LOG("Esecuzione logica interna... sta per essere lanciata un'eccezione.");
    throw std::runtime_error("Qualcosa e' andato storto!"); 
}

int main() 
{
    LOG("=== Unione dei Flussi e Sicurezza ===");

    // SEZIONE 1: Monitoraggio ciclo di vita di joinable()
    std::thread stateThread;
    LOG("Stato iniziale (non inizializzato) -> joinable(): " << (stateThread.joinable() ? "true" : "false"));

    stateThread = std::thread(processLongTask);
    LOG("Stato attivo (in esecuzione)       -> joinable(): " << (stateThread.joinable() ? "true" : "false"));

    stateThread.join();
    LOG("Stato rilasciato (dopo join)       -> joinable(): " << (stateThread.joinable() ? "true" : "false"));

    // SEZIONE 2: Rischi di Detach & Lifespan
    {
        std::thread detachedThread([]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        });
        detachedThread.detach(); 
        LOG("Thread distaccato con successo via detach().");
    }

    // SEZIONE 3: Protezione da eccezioni via RAII (ThreadGuard)
    try 
    {
        executeRiskyOperation();
    } 
    catch (const std::exception& e) 
    {
        LOG("Main ha catturato l'eccezione: '" << e.what() << "'. Il programma e' salvo grazie a RAII!");
    }

    return 0;
}