/*
 * ============================================================================
 * ESERCIZIO: Concurrent Read-Write Access with std::shared_mutex
 * ============================================================================
 * SCOPO:
 * Dimostrare il pattern Read-Write Lock utilizzando `std::shared_mutex` (C++17). 
 * Consente letture simultanee non bloccanti tra piu thread tramite `std::shared_lock`, 
 * garantendo al contempo l'accesso esclusivo in scrittura mediante `std::unique_lock` 
 * per evitare Data Race durante le modifiche alla mappa condivisa.
 *
 * STEP:
 * 1. Definire la risorsa condivisa (`userProfilesDatabase`) e il relativo `std::shared_mutex`.
 * 2. Implementare la task `readUserProfileTask` che acquisisce un lock condiviso (`std::shared_lock`).
 * 3. Implementare la task `updateUserProfileTask` che acquisisce un lock esclusivo (`std::unique_lock`).
 * 4. Istanziare molteplici thread lettori e scrittori nel `main` ed attenderne il completamento tramite `.join()`.
 *
 * Tags: std::thread, std::shared_mutex, std::shared_lock, std::unique_lock, read-write-lock, thread-safety
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <mutex>
#include <shared_mutex>
#include <thread>

// Risorsa condivisa
std::unordered_map<std::string, std::string> userProfilesDatabase;

// Mutex condiviso per la sincronizzazione Read-Write
std::shared_mutex databaseSharedMutex;

// --- FUNZIONE DI LETTURA (Letture concorrenti consentite) ---
void readUserProfileTask(const std::string& username, int readerThreadId)
{
    // Lock CONDIVISO: Consente accessi simultanei ad altri lettori
    std::shared_lock<std::shared_mutex> sharedLock(databaseSharedMutex);

    LOG("[Lettore " << readerThreadId << "] Sto leggendo il profilo di " << username);
    
    // Simulazione del tempo di lettura
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    const auto searchIterator = userProfilesDatabase.find(username);
    if (searchIterator != userProfilesDatabase.end()) 
    {
        LOG("[Lettore " << readerThreadId << "] Risultato: " << username << " -> " << searchIterator->second);
    } 
    else 
    {
        LOG("[Lettore " << readerThreadId << "] Risultato: Profilo di " << username << " non trovato.");
    }
}

// --- FUNZIONE DI SCRITTURA (Accesso esclusivo, blocca sia lettori che scrittori) ---
void updateUserProfileTask(const std::string& username, const std::string& newBiography, int writerThreadId)
{
    // Lock ESCLUSIVO: Blocca ogni altro thread durante la modifica
    std::unique_lock<std::shared_mutex> exclusiveLock(databaseSharedMutex);

    LOG("[Scrittore " << writerThreadId << "] !!! STO AGGIORNANDO !!! il profilo di " << username);
    
    // Simulazione dell'operazione di scrittura
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    userProfilesDatabase[username] = newBiography;

    LOG("[Scrittore " << writerThreadId << "] Aggiornamento completato.");
}

int main()
{
    // Popolamento iniziale del database
    userProfilesDatabase["mario89"] = "Sviluppatore C++";
    userProfilesDatabase["lucia_green"] = "Data Scientist";

    std::vector<std::thread> workerThreads;

    // Creazione del primo gruppo di lettori concorrenti
    for (int readerIndex = 1; readerIndex <= 5; ++readerIndex) 
    {
        workerThreads.emplace_back(readUserProfileTask, "mario89", readerIndex);
    }

    // Creazione degli scrittori
    workerThreads.emplace_back(updateUserProfileTask, "mario89", "Lead Software Engineer", 1);
    workerThreads.emplace_back(updateUserProfileTask, "lucia_green", "Senior AI Researcher", 2);

    // Creazione del secondo gruppo di lettori
    for (int readerIndex = 6; readerIndex <= 8; ++readerIndex) 
    {
        workerThreads.emplace_back(readUserProfileTask, "mario89", readerIndex);
    }

    // Attesa del completamento di tutti i thread
    for (auto& workerThread : workerThreads) 
    {
        if (workerThread.joinable()) 
        {
            workerThread.join();
        }
    }

    LOG("Tutti i thread hanno terminato.");
    return 0;
}