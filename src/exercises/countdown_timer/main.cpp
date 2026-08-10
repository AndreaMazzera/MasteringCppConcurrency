/*
 * ============================================================================
 * ESERCIZIO: Countdown Timer
 * ============================================================================
 * SCOPO:
 * Implementare un timer in background eseguito su un thread dedicato, in grado 
 * di gestire un polling reattivo tramite un flag atomico (std::atomic<bool>) 
 * per consentire l'interruzione anticipata e pulita del task.
 *
 * STEP:
 * 1. Avviare un thread dedicato per il countdown passando il numero di secondi e il flag atomico via std::cref.
 * 2. Nel loop del timer, verificare a ogni secondo lo stato di cancelFlag per interrompere l'attesa se richiesto.
 * 3. Eseguire un primo test completo per mostrare la naturale scadenza del timer.
 * 4. Eseguire un secondo test interrompendo il countdown a metà strada impostando cancelFlag a true dal thread principale.
 *
 * Tags: std::thread, std::atomic
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>

void startCountdown(int seconds, const std::atomic<bool>& cancelFlag)
{
    LOG("[Timer] Countdown avviato per " << seconds << " secondi.");

    for (int current = seconds; current > 0; --current)
    {
        if (cancelFlag.load())
        {
            LOG("[Timer] ❌ Timer annullato a T-minus " << current << "s!");
            return;
        }

        LOG("[Timer] ⏱️ T-minus " << current << "s...");
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    if (!cancelFlag.load())
    {
        LOG("[Timer] TEMPO SCADUTO! Din Don!");
    }
}

int main()
{
    LOG("=== BENCHMARK: COUNTDOWN TIMER CON THREAD ===");

    // ----------------------------------------------------
    // TEST 1: Countdown completo (3 secondi)
    // ----------------------------------------------------
    LOG("\n--- TEST 1: Countdown completo (3 secondi) ---");
    std::atomic<bool> cancelTest1{false};
    
    std::thread timer1(startCountdown, 3, std::cref(cancelTest1));

    LOG("[Main] Il timer sta girando in background, io proseguo...");
    
    if (timer1.joinable())
    {
        timer1.join();
    }

    // ----------------------------------------------------
    // TEST 2: Countdown annullato a metà strada
    // ----------------------------------------------------
    LOG("\n--- TEST 2: Countdown da 10s con annullamento a metà ---");
    std::atomic<bool> cancelTest2{false};

    std::thread timer2(startCountdown, 10, std::cref(cancelTest2));

    std::this_thread::sleep_for(std::chrono::milliseconds(2500));

    LOG("[Main] Decide di annullare il timer!");
    cancelTest2.store(true);

    if (timer2.joinable())
    {
        timer2.join();
    }

    LOG("\n=== Fine Esercizio ===");
    return 0;
}