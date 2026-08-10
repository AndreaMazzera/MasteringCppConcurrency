/*
 * ============================================================================
 * ESERCIZIO: Thread-Safe Cache with Readers-Writer Lock (std::shared_mutex)
 * ============================================================================
 * SCOPO:
 * Dimostrare l'implementazione di una cache in-memory thread-safe basata su 
 * `std::unordered_map` e gestita tramite un lock di tipo Readers-Writer (`std::shared_mutex`).
 * Illustra la combinazione di `std::shared_lock` (letture simultanee non bloccanti tra loro) 
 * e `std::unique_lock` (scritture esclusive), garantendo performance ottimali per scenari 
 * con un elevato rapporto di letture rispetto alle scritture.
 *
 * STEP:
 * 1. Implementare la classe `ThreadSafeCache` con `fetchValue` (lettura) e `insertOrUpdateValue` (scrittura).
 * 2. Utilizzare la parola chiave `mutable` per consentire l'acquisizione del lock `std::shared_lock` nei metodi `const`.
 * 3. Definire le task `readerWorkerTask` e `writerWorkerTask` per simulare l'accesso concorrente.
 * 4. Avviare un pool di thread nel `main` passando l'istanza della cache per riferimento via `std::cref` o `std::ref`.
 * 5. Sincronizzare la terminazione tramite `.join()` e stampare lo stato finale del dizionario.
 *
 * Tags: std::thread, std-shared-mutex, std-shared-lock, std-unique-lock, readers-writer-lock, thread-safe-cache, concurrency
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <string>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <optional>
#include <vector>
#include <thread>

class ThreadSafeCache 
{
public:
    ThreadSafeCache() = default;

    // METODO DI LETTURA (Shared Lock: Accesso simultaneo consentito a molteplici lettori)
    std::optional<std::string> fetchValue(const std::string& key) const 
    {
        std::shared_lock<std::shared_mutex> readLockGuard(sharedResourceMutex);

        auto searchIterator = cachedEntriesMap.find(key);
        if (searchIterator != cachedEntriesMap.end()) 
        {
            return searchIterator->second;
        }
        return std::nullopt; // Chiave non presente
    }

    // METODO DI SCRITTURA (Exclusive Lock: Accesso esclusivo riservato a un singolo scrittore)
    void insertOrUpdateValue(const std::string& key, const std::string& value) 
    {
        std::unique_lock<std::shared_mutex> writeLockGuard(sharedResourceMutex);
        cachedEntriesMap[key] = value;
    }

    // Stampa thread-safe dell'intero contenuto della cache
    void displayAllEntries() const 
    {
        std::shared_lock<std::shared_mutex> readLockGuard(sharedResourceMutex);
        for (const auto& [key, value] : cachedEntriesMap) 
        {
            LOG("Key: " << key << " | Value: " << value);
        }
    }

private:
    // 'mutable' consente l'acquisizione di un lock condiviso anche all'interno di metodi const
    mutable std::shared_mutex sharedResourceMutex; 
    std::unordered_map<std::string, std::string> cachedEntriesMap;
};

// Task per l'accesso in lettura
void readerWorkerTask(const ThreadSafeCache& cacheInstance, int workerId, const std::string& targetKey) 
{
    auto retrievedValue = cacheInstance.fetchValue(targetKey);
    if (retrievedValue.has_value()) 
    {
        LOG("Reader [" << workerId << "] -> Key: " << targetKey << " = " << *retrievedValue);
    } 
    else 
    {
        LOG("Reader [" << workerId << "] -> Key: " << targetKey << " NON TROVATA");
    }
}

// Task per l'accesso in scrittura
void writerWorkerTask(ThreadSafeCache& cacheInstance, int workerId, const std::string& targetKey, const std::string& valueToInsert) 
{
    cacheInstance.insertOrUpdateValue(targetKey, valueToInsert);
    LOG("Writer [" << workerId << "] -> Scritto (" << targetKey << " : " << valueToInsert << ")");
}

int main() 
{
    LOG("Core logici disponibili: " << std::thread::hardware_concurrency());

    ThreadSafeCache cacheInstance;

    // Inizializzazione dati
    cacheInstance.insertOrUpdateValue("db_host", "localhost");
    cacheInstance.insertOrUpdateValue("db_port", "5432");
    cacheInstance.insertOrUpdateValue("db_user", "admin");
    cacheInstance.insertOrUpdateValue("app_version", "1.0.0");

    constexpr int totalWorkerCount = 10;
    std::vector<std::thread> workerThreads;
    workerThreads.reserve(totalWorkerCount);

    // Avvio di 10 thread che alternano operazioni di lettura e scrittura
    for (int threadIndex = 0; threadIndex < totalWorkerCount; ++threadIndex) 
    {
        if (threadIndex % 2 == 0) 
        {
            // std::cref per la trasmissione sicura della cache const per riferimento
            workerThreads.emplace_back(readerWorkerTask, std::cref(cacheInstance), threadIndex, "db_port");
        } 
        else 
        {
            // std::ref per consentire la modifica dello stato interno
            workerThreads.emplace_back(writerWorkerTask, std::ref(cacheInstance), threadIndex, 
                                       "param_" + std::to_string(threadIndex), 
                                       "valore_" + std::to_string(threadIndex));
        }
    }

    // Join ordinato di tutti i worker thread
    for (auto& workerThread : workerThreads) 
    {
        if (workerThread.joinable()) 
        {
            workerThread.join();
        }
    }

    LOG("--- CONTENUTO FINALE DELLA CACHE ---");
    cacheInstance.displayAllEntries();

    return 0;
}