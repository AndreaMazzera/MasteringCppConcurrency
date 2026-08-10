/*
 * ============================================================================
 * ESERCIZIO: Dynamic Core Allocation
 * ============================================================================
 * SCOPO:
 * Ripartire in modo dinamico un carico di lavoro (batch di elementi) tra un 
 * numero sotto-dimensionato di thread calcolato a runtime in base ai core 
 * fisici disponibili (`std::thread::hardware_concurrency`), evitando di saturare 
 * l'intera CPU.
 *
 * STEP:
 * 1. Rilevare il numero di core della macchina e calcolare una quota di core da riservare al task.
 * 2. Dividere il range totale di elementi (es. 100 immagini) tra i thread allocati, gestendo i resti sull'ultimo thread.
 * 3. Assegnare a ciascun thread il sotto-insieme di dati da elaborare.
 * 4. Sincronizzare l'attesa di tutti i thread tramite .join().
 *
 * Tags: std::thread, std::thread::hardware_concurrency
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <thread>

void fakeImageProcessing(int workerId, int start_idx, int end_idx)
{
    LOG("[Worker " << workerId << "] Avviato per range immagini: " << start_idx << " a " << end_idx);
}

int main() 
{
    LOG("=== Parallel Image Processing Saggio ===");

    const int total_images = 100;
    int hardware_cores = std::thread::hardware_concurrency();
    LOG("Core totali rilevati sulla macchina: " << hardware_cores);
    
    // Strategia di allocazione: usiamo metà dei core disponibili (almeno 1)
    int cores_to_use = std::max(1, static_cast<int>(hardware_cores / 2));

    LOG("Core effettivamente allocati per questo task: " << cores_to_use);
   
    std::vector<std::thread> threads;
    threads.reserve(cores_to_use);
    
    int images_per_core = total_images / cores_to_use;

    for (int i = 0; i < cores_to_use; ++i)
    {
        int start_idx = i * images_per_core;
        
        // L'ultimo thread si fa carico dell'eventuale resto della divisione
        int end_idx = (i == cores_to_use - 1) ? (total_images - 1) : (start_idx + images_per_core - 1);
        
        threads.emplace_back(fakeImageProcessing, i, start_idx, end_idx);
    }

    for (auto& t : threads)
    {
        if (t.joinable()) t.join();
    }

    LOG("Tutte le immagini sono state elaborate senza fondere la CPU!");
    return 0;
}