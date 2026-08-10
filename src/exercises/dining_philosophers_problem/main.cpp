/*
 * ============================================================================
 * ESERCIZIO: Dining Philosophers (Cena dei Filosofi)
 * ============================================================================
 * SCOPO:
 * Risolvere il classico problema della Cena dei Filosofi evitando condizioni 
 * di Deadlock e Starvation mediante l'acquisizione atomica di risorse adiacenti 
 * tramite std::scoped_lock (C++17).
 *
 * STEP:
 * 1. Definire un array di 5 mutex globali per rappresentare le forchette condivise.
 * 2. Avviare 5 thread rappresentanti i filosofi che alternano fasi di pensiero e di pasto.
 * 3. Utilizzare uno scope locale ed acquisire atomicamente le due forchette adiacenti con std::scoped_lock.
 * 4. Rilasciare le risorse mediante RAII alla chiusura dello scope e attendere il completamento di tutti i thread con .join().
 *
 * Tags: std::thread, std::mutex, std::scoped_lock
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>

std::mutex chopsticks[5];

void eatSpaghetti(int id)
{
    for (int i = 0; i < 3; ++i)
    {
        LOG("[Filosofo " << id << "] Sta pensando... (Ciclo " << i + 1 << ")");
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        {
            // Acquisizione atomica delle due forchette adiacenti per prevenire deadlock
            std::scoped_lock lock(chopsticks[id], chopsticks[(id + 1) % 5]);
            
            LOG(" -> [Filosofo " << id << "] HA PRESO LE FORCHETTE E STA MANGIANDO");
            std::this_thread::sleep_for(std::chrono::milliseconds(60));
        }

        LOG(" <- [Filosofo " << id << "] Ha posato le forchette.");
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    LOG("[Filosofo " << id << "] HA FINITO DI CENARE E SE NE VA.");
}

int main() 
{
    LOG("Core logici disponibili: " << std::thread::hardware_concurrency());
    LOG("=== Inizio della cena dei 5 Filosofi ===");
    LOG("------------------------------------------------");
    
    std::vector<std::thread> threads;
    threads.reserve(5);

    for (int i = 0; i < 5; ++i)
    {
        threads.emplace_back(eatSpaghetti, i);
    }

    for (auto& t : threads)
    {
        if (t.joinable())
        {
            t.join();
        }
    }

    LOG("------------------------------------------------");
    LOG("=== La cena è terminata senza alcun deadlock! ===");
    return 0;
}