/*
 * ============================================================================
 * ESERCIZIO: Encapsulated Shared Configuration Registry with std::shared_mutex
 * ============================================================================
 * SCOPO:
 * Dimostrare l'incapsulamento di un registro di configurazione thread-safe 
 * (`SharedConfigRegistry`) utilizzando `std::shared_mutex` e `mutable`. 
 * Consente l'accesso in sola lettura tramite metodi `const` sfruttando `std::shared_lock` 
 * e l'aggiornamento esclusivo tramite `std::unique_lock`.
 *
 * STEP:
 * 1. Definire la classe `SharedConfigRegistry` con membri privati `configMap` e `registrySharedMutex` (`mutable`).
 * 2. Implementare il metodo `const` `getObject` per letture concorrenti trasparenti.
 * 3. Implementare il metodo `updateSetting` per le modifiche esclusive.
 * 4. Istanziare le task worker per lettori (`readerTask`) e scrittori (`writerTask`) e gestire il pool nel `main` via `std::cref` e `std::ref`.
 *
 * Tags: std::thread, std::shared_mutex, std::shared_lock, std::unique_lock, const-correctness, thread-safe-registry
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <chrono>
#include <mutex>
#include <shared_mutex>
#include <thread>

class SharedConfigRegistry 
{
public:
    SharedConfigRegistry() = default;

    std::optional<std::string> getObject(const std::string& key) const
    {
        std::shared_lock<std::shared_mutex> sharedLock(registrySharedMutex);
        
        // Simulazione del carico di lettura all'interno della sezione critica
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        const auto mapIterator = configMap.find(key);
        if (mapIterator != configMap.end())
        {
            return mapIterator->second;
        }
        return std::nullopt;
    }

    void updateSetting(const std::string& key, const std::string& value) 
    {
        std::unique_lock<std::shared_mutex> exclusiveLock(registrySharedMutex);
        
        // Simulazione del carico di scrittura all'interno della sezione critica
        std::this_thread::sleep_for(std::chrono::milliseconds(300));

        configMap[key] = value;
    }

private:
    std::unordered_map<std::string, std::string> configMap;
    mutable std::shared_mutex registrySharedMutex; // Consentito l'uso nei metodi const
};

// --- FUNZIONI WORKER DEI THREAD ---

void readerTask(int readerThreadId, const SharedConfigRegistry& registry)
{
    auto getCurrentTimestampMs = []() { 
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count() % 100000; 
    };

    LOG("[Lettore " << readerThreadId << "] Tentativo di lettura al timestamp: " << getCurrentTimestampMs() << " ms");
    
    const auto retrievedValue = registry.getObject("timeout");
    
    if (retrievedValue)
    {
        LOG("[Lettore " << readerThreadId << "] Lettura completata! Valore: " << *retrievedValue << " al timestamp: " << getCurrentTimestampMs() << " ms");
    }
    else
    {
        LOG("[Lettore " << readerThreadId << "] Lettura completata! Chiave non trovata al timestamp: " << getCurrentTimestampMs() << " ms");
    }
}

void writerTask(int writerThreadId, SharedConfigRegistry& registry, std::string newSettingValue)
{
    auto getCurrentTimestampMs = []() { 
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count() % 100000; 
    };

    LOG("[Scrittore " << writerThreadId << "] Tentativo di SCRITTURA al timestamp: " << getCurrentTimestampMs() << " ms");
    
    registry.updateSetting("timeout", newSettingValue);
    
    LOG("[Scrittore " << writerThreadId << "] SCRITTURA completata! Nuovo valore: " << newSettingValue << " al timestamp: " << getCurrentTimestampMs() << " ms");
}

int main() 
{
    LOG("Core logici disponibili: " << std::thread::hardware_concurrency());
    
    SharedConfigRegistry registry;
    
    // Inizializzazione del registro con un valore di default
    registry.updateSetting("timeout", "30");

    std::vector<std::thread> workerThreads;
    workerThreads.reserve(10); // Pre-allocazione memoria del vettore

    LOG("=== Avvio simulazione Readers-Writers ===");

    // Creazione di 8 thread lettori
    for (int readerIndex = 0; readerIndex < 8; ++readerIndex)
    {
        workerThreads.emplace_back(readerTask, readerIndex + 1, std::cref(registry));
    }

    // Creazione di 2 thread scrittori
    for (int writerIndex = 0; writerIndex < 2; ++writerIndex)
    {
        workerThreads.emplace_back(writerTask, writerIndex + 1, std::ref(registry), "10" + std::to_string(writerIndex));
    }

    // Attesa del completamento di tutti i thread
    for (auto& workerThread : workerThreads)
    {
        if (workerThread.joinable())
        {
            workerThread.join();
        }
    }

    LOG("=== Fine simulazione ===");
    return 0;
}