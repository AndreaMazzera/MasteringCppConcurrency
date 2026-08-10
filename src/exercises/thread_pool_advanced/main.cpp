/*
 * ============================================================================
 * ESERCIZIO: Work-Stealing ThreadPool Pattern Implementation
 * ============================================================================
 * SCOPO:
 * Dimostrare l'implementazione del pattern Work-Stealing per un ThreadPool multithread.
 * Ogni worker thread possiede una propria coda locale `WorkStealingQueue` basata su `std::deque`.
 * Il thread proprietario gestisce i propri task in modalità LIFO (push/pop dalla TESTA),
 * mentre gli altri worker thread (ladri) rubano task in modalità FIFO (steal dal FONDO)
 * per bilanciare dinamicamente il carico di lavoro senza contesa centralizzata.
 *
 * STEP:
 * 1. Implementare `WorkStealingQueue` con operazioni thread-safe per push, pop (LIFO) e steal (FIFO).
 * 2. Inizializzare un vettore di `WorkStealingQueue` ed avviare N worker thread con `threadLocalWorkerIndex`.
 * 3. Implementare `workerLoopTask` che priorotizza l'estrazione dalla propria coda e tenta lo steal in caso di coda vuota.
 * 4. Implementare `submitTask` distribuendo via round-robin per chiamate esterne o nella propria coda se da worker interno.
 * 5. Gestire il shutdown pulito nel distruttore via `isShutdownRequested` e `.join()`.
 *
 * Tags: std::thread, work-stealing, thread-pool, lock-free-concepts, load-balancing, thread-local-storage, concurrency
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <chrono>
#include <deque>
#include <memory>
#include <atomic>
#include <thread>

using TaskFunction = std::function<void()>;

class WorkStealingQueue
{
public:
    WorkStealingQueue() = default;

    // Disabilitazione della copia della coda
    WorkStealingQueue(const WorkStealingQueue&) = delete;
    WorkStealingQueue& operator=(const WorkStealingQueue&) = delete;

    // Inserimento dalla TESTA da parte del thread proprietario
    void pushToHead(TaskFunction task)
    {
        std::lock_guard<std::mutex> lockGuard(queueMutex);
        taskDeque.push_front(std::move(task));
    }

    // Estrazione dalla TESTA da parte del thread proprietario (LIFO)
    bool popFromHead(TaskFunction& task)
    {
        std::lock_guard<std::mutex> lockGuard(queueMutex);
        if (taskDeque.empty()) 
        {
            return false;
        }

        task = std::move(taskDeque.front());
        taskDeque.pop_front();
        return true;
    }

    // Estrazione dal FONDO da parte di un altro worker thread (FIFO / Steal)
    bool stealFromTail(TaskFunction& task)
    {
        std::lock_guard<std::mutex> lockGuard(queueMutex);
        if (taskDeque.empty()) 
        {
            return false;
        }

        task = std::move(taskDeque.back());
        taskDeque.pop_back();
        return true;
    }

    bool isQueueEmpty()
    {
        std::lock_guard<std::mutex> lockGuard(queueMutex);
        return taskDeque.empty();
    }

private:
    std::deque<TaskFunction> taskDeque;
    std::mutex queueMutex;
};

class WorkStealingThreadPool 
{
public:
    explicit WorkStealingThreadPool(size_t totalWorkerThreads) : isShutdownRequested(false)
    {
        // 1. Inizializzazione preliminare delle code locali
        workerQueues.reserve(totalWorkerThreads);
        for (size_t queueIndex = 0; queueIndex < totalWorkerThreads; ++queueIndex) 
        {
            workerQueues.push_back(std::make_unique<WorkStealingQueue>());
        }

        // 2. Avvio dei thread worker
        workerThreads.reserve(totalWorkerThreads);
        for (size_t threadIndex = 0; threadIndex < totalWorkerThreads; ++threadIndex) 
        {
            workerThreads.emplace_back(&WorkStealingThreadPool::workerLoopTask, this, static_cast<int>(threadIndex));
        }
    }

    void workerLoopTask(int currentWorkerIndex)
    {
        threadLocalWorkerIndex = currentWorkerIndex;

        while (!isShutdownRequested)
        {
            TaskFunction currentTask;
            bool isTaskRetrieved = false;

            // Step 1: Prova ad estrarre dalla propria coda locale (TESTA - LIFO)
            if (workerQueues[currentWorkerIndex]->popFromHead(currentTask)) 
            {
                isTaskRetrieved = true;
            } 
            else 
            {
                // Step 2: In caso di coda vuota, tenta lo Steal dal FONDO delle code altrui (FIFO)
                const size_t totalWorkersCount = workerThreads.size();
                for (size_t searchIndex = 0; searchIndex < totalWorkersCount; ++searchIndex) 
                {
                    const size_t targetWorkerIndex = (currentWorkerIndex + searchIndex + 1) % totalWorkersCount;
                    
                    if (workerQueues[targetWorkerIndex]->stealFromTail(currentTask)) 
                    {
                        LOG("[Worker " << currentWorkerIndex << "] Ha RUBAVO un task da Worker " << targetWorkerIndex);
                        isTaskRetrieved = true;
                        break;
                    }
                }
            }

            // Step 3: Esecuzione del task se recuperato, altrimenti cede la CPU
            if (isTaskRetrieved) 
            {
                currentTask();
            } 
            else 
            {
                std::this_thread::yield();
            }
        }
    }

    void submitTask(TaskFunction taskToSubmit) 
    {
        if (isShutdownRequested) 
        {
            throw std::runtime_error("Impossibile inserire task in un ThreadPool spento!");
        }

        if (threadLocalWorkerIndex != -1) 
        {
            // Se la sottomissione avviene all'interno di un worker, inserisce nella propria coda
            workerQueues[threadLocalWorkerIndex]->pushToHead(std::move(taskToSubmit));
        }
        else 
        {
            // Se chiamata dall'esterno, distribuisce i task in Round-Robin tra le code
            const size_t targetQueueIndex = nextQueueAssignmentIndex++ % workerThreads.size();
            workerQueues[targetQueueIndex]->pushToHead(std::move(taskToSubmit));  
        }
    }

    ~WorkStealingThreadPool() 
    {
        isShutdownRequested = true;
        for (std::thread& workerThread : workerThreads) 
        {
            if (workerThread.joinable()) 
            {
                workerThread.join();
            }
        }
    }

    WorkStealingThreadPool(const WorkStealingThreadPool&) = delete;
    WorkStealingThreadPool& operator=(const WorkStealingThreadPool&) = delete;

private:
    std::vector<std::unique_ptr<WorkStealingQueue>> workerQueues;
    std::vector<std::thread> workerThreads;
    std::atomic<bool> isShutdownRequested{false};
    std::atomic<size_t> nextQueueAssignmentIndex{0}; 
    
    static inline thread_local int threadLocalWorkerIndex{-1};
};

int main() 
{
    LOG("Avvio del Work-Stealing ThreadPool con 4 worker...");

    constexpr size_t workerThreadsCount = 4;
    constexpr int totalTasksSubmittedCount = 12;

    WorkStealingThreadPool threadPool(workerThreadsCount);

    // Invio dei task al pool
    for (int taskIndex = 1; taskIndex <= totalTasksSubmittedCount; ++taskIndex) 
    {
        threadPool.submitTask([taskIndex]() {
            LOG("Esecuzione Task " << taskIndex << " inizio...");
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            LOG("Esecuzione Task " << taskIndex << " COMPLETATO!");
        });
    }

    // Tempo sufficiente all'esecuzione dei task nel pool prima del shutdown
    std::this_thread::sleep_for(std::chrono::seconds(2));

    LOG("Main terminato.");
    return 0;
}