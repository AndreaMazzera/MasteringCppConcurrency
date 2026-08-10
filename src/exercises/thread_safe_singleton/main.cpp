/*
 * ============================================================================
 * ESERCIZIO: Thread-Safe Meyers' Singleton Pattern Implementation
 * ============================================================================
 * SCOPO:
 * Dimostrare l'implementazione del pattern Singleton thread-safe (Meyers' Singleton) 
 * sfruttando le garanzie dello standard C++11 (e successivi), in cui l'inizializzazione 
 * delle variabili statiche locali a blocco e thread-safe per definizione di linguaggio.
 * Evita l'uso esplicito di mutex o `std::call_once` durante la prima istanziazione.
 *
 * STEP:
 * 1. Definire la classe `ConfigurationManager` con costruttore privato e disabilitazione di copia ed assegnazione.
 * 2. Implementare `getInstance` con una variabile statica locale `singletonInstance`.
 * 3. Implementare `fetchValue` per la lettura immutabile di configurazioni memorizzate in `std::unordered_map`.
 * 4. Avviare un pool di worker thread nel `main` che accedono concorrentemente all'unica istanza di Singleton.
 * 5. Verificare che l'indirizzo dell'istanza rimanga identico per tutti i thread e completare con `.join()`.
 *
 * Tags: std::thread, meyers-singleton, thread-safe-singleton, static-initialization, design-patterns, concurrency
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <thread>

class ConfigurationManager 
{
public:
    // Accesso al Meyers' Singleton (Thread-Safe nativo C++11)
    static ConfigurationManager& getInstance() 
    {
        static ConfigurationManager singletonInstance;
        return singletonInstance;
    }

    // Lettura thread-safe delle configurazioni (sola lettura post-inizializzazione)
    std::string fetchValue(const std::string& targetKey) const 
    {
        auto searchIterator = configurationMap.find(targetKey);
        if (searchIterator != configurationMap.end()) 
        {
            return searchIterator->second;
        }
        return "NOT_FOUND";
    }

    // Disabilitazione della copia e dell'assegnazione
    ConfigurationManager(const ConfigurationManager&) = delete;
    ConfigurationManager& operator=(const ConfigurationManager&) = delete;

private:
    std::unordered_map<std::string, std::string> configurationMap;

    // Costruttore privato per impedire istanziazioni esterne
    ConfigurationManager() 
    {
        LOG("=== Inizializzazione Unica ConfigurationManager (Costruttore) ===");
        
        // Caricamento e popolazione iniziale dei parametri di configurazione
        configurationMap["db_host"] = "localhost";
        configurationMap["db_port"] = "5432";
        configurationMap["db_user"] = "admin";
        configurationMap["app_version"] = "1.0.0";
    }
};

void workerTask(int workerId) 
{
    // Accesso concorrente all'unica istanza gestita dal linguaggio
    auto& configurationManager = ConfigurationManager::getInstance();
    
    // Recupero del valore relativo alla chiave richiesta
    const std::string databaseHost = configurationManager.fetchValue("db_host");
    
    LOG("Thread [" << workerId << "] -> Indirizzo Istanza: " << &configurationManager 
        << " | db_host = " << databaseHost);
}

int main() 
{
    LOG("Core logici disponibili: " << std::thread::hardware_concurrency());

    constexpr int totalWorkerCount = 5;
    std::vector<std::thread> workerThreads;
    workerThreads.reserve(totalWorkerCount);

    // Lancio dei thread worker concorrenti
    for (int threadIndex = 0; threadIndex < totalWorkerCount; ++threadIndex)
    {
        workerThreads.emplace_back(workerTask, threadIndex);
    }

    // Attesa del completamento di tutti i worker thread
    for (auto& workerThread : workerThreads)
    {
        if (workerThread.joinable())
        {
            workerThread.join();
        }
    }
    
    LOG("=== Fine esecuzione ===");
    return 0;
}