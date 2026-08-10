/*
 * ============================================================================
 * ESERCIZIO: Join vs Detach
 * ============================================================================
 * SCOPO:
 * Mostrare la differenza di comportamento tra std::thread::join() (attesa 
 * bloccante del completamento) e std::thread::detach() (esecuzione del task 
 * in background disaccoppiata dal ciclo di vita del thread principale).
 *
 * STEP:
 * 1. Avviare un task critico (controllo motori) su threadMotor e un task secondario su threadMusic.
 * 2. Eseguire .detach() su threadMusic per lasciarlo girare in background in autonomia.
 * 3. Eseguire .join() su threadMotor per garantire il completamento del controllo prima del decollo.
 * 4. Verificare che la chiusura del thread principale proceda senza sollevare eccezioni o std::terminate().
 *
 * Tags: std::thread, std::thread::join, std::thread::detach, std::thread::joinable
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <string>
#include <chrono>
#include <thread>

void controlloMotori()
{
    LOG("Inizio controllo motori...");
    std::this_thread::sleep_for(std::chrono::seconds(2));
    LOG("Controllo motori superato!");
}

void riproduciMusica()
{
    for (int i = 0; i < 5; ++i)
    {
        LOG("~ Riproduzione musica di sottofondo... ~");
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

int main() 
{
    LOG("Core logici disponibili: " << std::thread::hardware_concurrency());
    
    std::thread threadMotor(controlloMotori);
    std::thread threadMusic(riproduciMusica);

    // Detach del task non critico: proseguirà in background
    if (threadMusic.joinable())
    {
        threadMusic.detach();
    }

    // Join del task critico: il main DEVE attendere questo controllo
    if (threadMotor.joinable())
    {
        threadMotor.join();
    }

    LOG("DECOLLO AVVENUTO CON SUCCESSO!");

    return 0;
}