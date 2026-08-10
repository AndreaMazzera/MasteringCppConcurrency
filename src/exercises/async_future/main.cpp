/*
 * ============================================================================
 * ESERCIZIO: Il Trittico del Calcolo Asincrono (Promise, Packaged Task, Async)
 * ============================================================================
 * SCOPO:
 * Esplorare e mettere a confronto le tre strategie fondamentali del C++ moderno
 * per l'esecuzione di task in background e il recupero asincrono dei risultati 
 * tramite std::future, partendo dai meccanismi manuali di basso livello fino 
 * alle astrazioni ad alto livello.
 *
 * MODELLI E ARCHITETTURA:
 * 
 * 1. std::promise + std::future (Canale Manuale Punto-a-Punto)
 *    - Rappresenta l'estremità di "scrittura" di un canale di comunicazione asincrono.
 *    - Consente ad un thread worker di inviare un valore o un'eccezione in modo del tutto
 *      esplicito, disaccoppiando l'uscita del task dall'interfaccia della funzione stessa.
 *
 * 2. std::packaged_task + std::future (Task Disaccoppiato)
 *    - Incapsula qualsiasi callable (funzione, lambda, functor) connettendola a un future.
 *    - Separa nettamente la creazione del task dalla sua esecuzione temporale o spaziale
 *      (ideale per essere inserito nelle code dei Thread Pool).
 *
 * 3. std::async + std::future (Astrazione Completa)
 *    - Delega interamente al runtime la creazione del thread e la gestione del ciclo di vita.
 *    - Gestisce la sincronizzazione, il monitoraggio con timeout (wait_for) e la 
 *      propagazione trasparente delle eccezioni dal thread secondario al main.
 *
 * STEP DI ESECUZIONE:
 * 1. Instanziare un std::promise, passarlo ad un thread worker e consumare il valore prodotto.
 * 2. Creare un std::packaged_task, estrarne il future ed eseguirlo su un thread dedicato.
 * 3. Avviare un task ad alto livello con std::async, gestendone il timeout e le eccezioni.
 * 4. Recuperare tutti i risultati nel thread principale tramite le rispettive chiamate a .get().
 *
 * Tags: std::promise, std::packaged_task, std::async, std::future, std::launch::async, std::future_status
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <future>
#include <exception>
#include <stdexcept>

// ----------------------------------------------------------------------------
// 1. COMUNICAZIONE MANUALE CON std::promise
// ----------------------------------------------------------------------------
void workerWithPromise(std::promise<double> resultPromise)
{
    LOG("[Promise Worker] Avvio elaborazione complessa...");
    std::this_thread::sleep_for(std::chrono::milliseconds(800));

    try {
        // Simulazione del calcolo di un valore
        double value = 42.5;
        LOG("[Promise Worker] Invio risultato tramite promise...");
        resultPromise.set_value(value);
    } 
    catch (...) {
        // In caso di errore, si può trasmettere l'eccezione al future
        resultPromise.set_exception(std::current_exception());
    }
}

// ----------------------------------------------------------------------------
// 2. TASK DISACCOPPIATO CON std::packaged_task
// ----------------------------------------------------------------------------
int computeHash(const std::string& input)
{
    LOG("[PackagedTask Thread] Calcolo hash per: " << input);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    return static_cast<int>(input.length() * 1337);
}

// ----------------------------------------------------------------------------
// 3. TASK ASINCRONO CON std::async (Con gestione eccezioni)
// ----------------------------------------------------------------------------
std::string fetchDataWithAsync(int serverId)
{
    LOG("[Async Task] Connessione al server " << serverId << "...");
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    if (serverId < 0) {
        throw std::invalid_argument("ID Server non valido!");
    }

    return "Dati dal Server " + std::to_string(serverId);
}

int main()
{
    LOG("=== BENCHMARK COMPLETO: ADVANCED ASYNC & FUTURES ===");

    // ========================================================================
    // DEMO 1: std::promise & std::future
    // ========================================================================
    LOG("\n--- 1. Demo std::promise / std::future ---");

    std::promise<double> promiseObj;
    std::future<double> promiseFuture = promiseObj.get_future();

    std::thread promiseThread(workerWithPromise, std::move(promiseObj));

    LOG("[Main] In attesa del valore dal worker promise...");
    LOG("[Main] Risultato std::promise: " << promiseFuture.get());

    if (promiseThread.joinable()) {
        promiseThread.join();
    }


    // ========================================================================
    // DEMO 2: std::packaged_task
    // ========================================================================
    LOG("\n--- 2. Demo std::packaged_task ---");

    // Creo il task e ne estraggo il future PRIMA dell'esecuzione
    std::packaged_task<int(const std::string&)> task(computeHash);
    std::future<int> hashFuture = task.get_future();

    // Il task può essere eseguito su un thread dedicato o passato a un Thread Pool
    std::thread taskThread(std::move(task), "PayloadDati");
    taskThread.detach(); // O join() a seconda dell'architettura

    LOG("[Main] Risultato std::packaged_task: " << hashFuture.get());


    // ========================================================================
    // DEMO 3: std::async + Timeout (wait_for) + Exception Handling
    // ========================================================================
    LOG("\n--- 3. Demo std::async con timeout ed eccezione ---");
    
    std::future<std::string> asyncFuture = std::async(
        std::launch::async, 
        fetchDataWithAsync, 
        42
    );

    // Monitoraggio non bloccante dello stato del task
    while (asyncFuture.wait_for(std::chrono::milliseconds(400)) != std::future_status::ready) {
        LOG("[Main] asyncFuture non ancora pronto... Il Main continua l'elaborazione.");
    }

    // Recupero sicuro del valore
    LOG("[Main] Risultato std::async: " << asyncFuture.get());

    // Test propagazione eccezione
    std::future<std::string> errorFuture = std::async(
        std::launch::async, 
        fetchDataWithAsync, 
        -1 // Forzerà un'eccezione
    );

    try {
        errorFuture.get(); // Rilancia l'eccezione catturata nel thread secondario
    } 
    catch (const std::exception& e) {
        LOG("[Main] Eccezione catturata con successo da async: " << e.what());
    }

    LOG("\n=== Fine dell'esercizio ===");
    return 0;
}