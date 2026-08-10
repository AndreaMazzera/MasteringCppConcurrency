/*
 * ============================================================================
 * ESERCIZIO: Custom ThreadPool Implementation with std::condition_variable
 * ============================================================================
 * SCOPO:
 * Dimostrare l'implementazione da zero di una classe `ThreadPool` basata su una 
 * coda di task thread-safe (`std::queue<std::function<void()>>`) sincronizzata via 
 * `std::condition_variable`. Illustra l'architettura Produttore-Consumatore, la 
 * gestione sicura del ciclo di vita dei worker thread ed il shutdown Graceful nel distruttore.
 *
 * STEP:
 * 1. Definire la classe `ThreadPool` contenente i thread worker, la coda task, il mutex e la condition variable.
 * 2. Nel costruttore, avviare N worker thread in un loop di attesa condizionata via `.wait()`.
 * 3. Implementare il metodo `enqueueTask` per aggiungere nuove lambdas alla coda e notificare un worker via `.notify_one()`.
 * 4. Gestire la distruzione sicura nel distruttore impostando `isShutdownRequested`, notificando tutti via `.notify_all()` e riorganizzando le risorse via `.join()`.
 *
 * Tags: std::thread, thread-pool, std-condition-variable, producer-consumer, thread-safety, graceful-shutdown
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <functional>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <thread>

class ThreadPool 
{
public:
    explicit ThreadPool(size_t workerThreadsCount) : isShutdownRequested(false) 
    {
        workerThreads.reserve(workerThreadsCount);
        
        for (size_t threadIndex = 0; threadIndex < workerThreadsCount; ++threadIndex) 
        {
            workerThreads.emplace_back([this, threadIndex]() {
                const size_t workerThreadId = threadIndex + 1;
                
                while (true) 
                {
                    std::function<void()> taskToExecute;

                    {
                        std::unique_lock<std::mutex> lockGuard(this->queueMutex);

                        // Attesa condizionata fino alla disponibilita di una task o alla richiesta di arresto
                        this->workAvailableCondition.wait(lockGuard, [this]() {
                            return this->isShutdownRequested || !this->taskQueue.empty();
                        });

                        // Condizione di uscita dal thread worker: arresto richiesto e coda svuotata
                        if (this->isShutdownRequested && this->taskQueue.empty()) 
                        {
                            return;
                        }

                        // Estrazione della task dalla coda (FIFO)
                        taskToExecute = std::move(this->taskQueue.front());
                        this->taskQueue.pop();
                    }

                    // Esecuzione del lavoro fuori dalla sezione critica (lock rilasciato)
                    taskToExecute();
                }
            });
        }
    }

    void enqueueTask(std::function<void()> taskFunction) 
    {
        {
            std::unique_lock<std::mutex> lockGuard(queueMutex);
            
            if (isShutdownRequested) 
            {
                throw std::runtime_error("Impossibile inserire task in un ThreadPool gia spento!");
            }
            
            taskQueue.push(std::move(taskFunction));
        }
        
        // Notifica ad un singolo worker thread in attesa
        workAvailableCondition.notify_one();
    }

    ~ThreadPool() 
    {
        {
            std::unique_lock<std::mutex> lockGuard(queueMutex);
            isShutdownRequested = true;
        }

        // Risveglio di tutti i thread worker per il processo di shutdown
        workAvailableCondition.notify_all();

        // Attesa sequenziale e chiusura pulita di tutti i worker thread
        for (std::thread& workerThread : workerThreads) 
        {
            if (workerThread.joinable()) 
            {
                workerThread.join();
            }
        }
    }

    // Disabilitazione della copia e dell'assegnazione
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

private:
    std::vector<std::thread> workerThreads;
    std::queue<std::function<void()>> taskQueue;

    std::mutex queueMutex;
    std::condition_variable workAvailableCondition;
    bool isShutdownRequested;
};

int main() 
{
    LOG("Avvio del ThreadPool con 4 worker...");

    constexpr size_t totalWorkersCount = 4;
    constexpr int totalTasksCount = 8;

    ThreadPool threadPool(totalWorkersCount);

    // Invio delle task al ThreadPool
    for (int taskIndex = 1; taskIndex <= totalTasksCount; ++taskIndex) 
    {
        threadPool.enqueueTask([taskIndex]() {
            LOG("Esecuzione Task " << taskIndex << " inizio...");
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            LOG("Esecuzione Task " << taskIndex << " COMPLETATO!");
        });
    }

    LOG("Tutti i task sono stati inseriti nella coda.");

    // Il distruttore di 'threadPool' gestira il completamento di tutte le task e il join dei thread
    return 0;
}