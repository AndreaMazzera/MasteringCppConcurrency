/*
 * ============================================================================
 * ESERCIZIO: Debug Deadlocks & scoped_lock
 * ============================================================================
 * SCOPO:
 * Dimostrare una condizione di Deadlock causata dall'acquisizione di mutex in
 * ordine incoerente tra thread diversi (Attesa Circolare) e mostrare la
 * risoluzione moderna tramite std::scoped_lock (C++17).
 *
 * STEP:
 * 1. Definire la struttura Account contenente un bilancio e un std::mutex associato.
 * 2. Creare la funzione buggyTransfer per mostrare come l'acquisizione sequenziale
 *    di due mutex distinti porti a un blocco indefinito se richiamata in ordine inverso.
 * 3. Implementare la funzione safeTransfer che utilizza std::scoped_lock per acquisire
 *    più mutex contemporaneamente tramite un algoritmo che previene i deadlock.
 * 4. Eseguire due trasferimenti concorrenti incrociati dimostrando la thread-safety.
 *
 * Tags: std::thread, std::mutex, std::unique_lock, std::scoped_lock
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>

struct Account 
{
    int mId;
    int mBalance;
    std::mutex mMtx;

    Account(int id, int balance) 
        : mId(id), mBalance(balance) {}
};

// --- VERSIONE ERRATA: Causa DEADLOCK (Attesa Circolare) ---
void buggyTransfer(Account& from, Account& to, int amount)
{
    LOG("Tento di acquisire lock per Account " << from.mId);
    std::unique_lock<std::mutex> lockFrom(from.mMtx);
    LOG("Lock acquisito per Account " << from.mId);

    // Simula context-switch per forzare l'attesa circolare tra i thread
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    LOG("Tento di acquisire lock per Account " << to.mId);
    std::unique_lock<std::mutex> lockTo(to.mMtx);
    LOG("Lock acquisito per Account " << to.mId);

    from.mBalance -= amount;
    to.mBalance += amount;

    LOG("Trasferimento completato!");
}

// --- VERSIONE CORRETTA: Thread-safe con std::scoped_lock (C++17) ---
void safeTransfer(Account& from, Account& to, int amount)
{
    // std::scoped_lock acquisisce N mutex evitando il deadlock a livello di algoritmo
    std::scoped_lock lock(from.mMtx, to.mMtx);

    from.mBalance -= amount;
    to.mBalance += amount;

    LOG("Trasferimento completato in sicurezza da Account " << from.mId << " ad Account " << to.mId);
}

int main()
{
    Account acc1(1, 1000);
    Account acc2(2, 500);

    /* 
    // Descommentare per simulare il deadlock:
    LOG("=== Avvio simulazione DEADLOCK (buggyTransfer) ===");
    std::thread t1_bug(buggyTransfer, std::ref(acc1), std::ref(acc2), 100);
    std::thread t2_bug(buggyTransfer, std::ref(acc2), std::ref(acc1), 50);

    t1_bug.join();
    t2_bug.join();
    */

    LOG("=== Test Trasferimento Sicuro (safeTransfer) ===");

    std::thread t1(safeTransfer, std::ref(acc1), std::ref(acc2), 100);
    std::thread t2(safeTransfer, std::ref(acc2), std::ref(acc1), 50);

    t1.join();
    t2.join();

    LOG("=== Test completato con successo ===");
    LOG("Saldo finale Acc1: " << acc1.mBalance << ", Acc2: " << acc2.mBalance);

    return 0;
}