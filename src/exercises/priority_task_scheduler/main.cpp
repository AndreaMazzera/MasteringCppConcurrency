/*
 * ============================================================================
 * ESERCIZIO: Thread Pool with Priority and Delay Task Scheduling
 * ============================================================================
 * SCOPO:
 * Dimostrare l'implementazione di uno scheduler di task concorrente basato su 
 * `std::priority_queue`. Supporta la prioritizzazione dei task (da LOW a CRITICAL) 
 * e la schedulazione temporizzata (invio ritardato via `wait_until`). Un pool di 
 * worker thread estrae ed esegue i task in ordine di priorita ed orario di avvio, 
 * rilasciando il mutex prima dell'esecuzione dell'operazione.
 *
 * STEP:
 * 1. Definire l'enum `Priority` e la struct `Task` con sovraccarico dell'operatore `<`.
 * 2. Implementare la classe `PriorityTaskScheduler` gestendo la coda di priorita, la sincronizzazione via `std::condition_variable` e il ciclo di vita dei thread (`workerLifecycleTask`).
 * 3. Supportare sia l'invio immediato (`submit`) che ritardato (`submitScheduled`).
 * 4. Testare l'invio concorrente di task a priorita differenti e verificare il rispetto delle precedenze temporali.
 *
 * Tags: std::thread, std::priority_queue, std::condition_variable, std::mutex, thread-pool, task-scheduler, delay-queue
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

enum class Priority
{
    LOW = 0, 
    MEDIUM = 1, 
    HIGH = 2, 
    CRITICAL = 3
};

struct Task
{
    std::function<void()> jobCallback;
    Priority taskPriority;
    std::chrono::steady_clock::time_point startTime;
    int taskId;

    bool operator<(const Task& other) const
    {
        if (taskPriority != other.taskPriority)
        {
            return taskPriority < other.taskPriority; 
        }
        return startTime > other.startTime;
    }
};

class PriorityTaskScheduler
{
public:
    explicit PriorityTaskScheduler(int numThreads)
    {
        workerThreads.reserve(numThreads);
        for (int i = 0; i < numThreads; ++i)
        {
            workerThreads.emplace_back(&PriorityTaskScheduler::workerLifecycleTask, this);
        }
    }

    ~PriorityTaskScheduler() 
    {
        {
            std::lock_guard<std::mutex> lock(schedulerMutex);
            isShutdownRequested = true;
        }
        conditionVar.notify_all();
        
        for (auto& threadWorker : workerThreads)
        {
            if (threadWorker.joinable())
            {
                threadWorker.join();
            }
        }
    }

    void workerLifecycleTask()
    {
        while (true)
        {
            Task taskToRun;
            {
                std::unique_lock<std::mutex> lock(schedulerMutex);

                // 1. Attesa finche non e presente un task o viene richiesto l'arresto
                conditionVar.wait(lock, [this]() {
                    return isShutdownRequested || !priorityTaskQueue.empty();
                });

                if (isShutdownRequested && priorityTaskQueue.empty())
                {
                    return;
                }

                // 2. Controllo del tempo di esecuzione per il task a priorita piu alta
                const auto currentTime = std::chrono::steady_clock::now();
                if (!priorityTaskQueue.empty() && priorityTaskQueue.top().startTime > currentTime)
                {
                    conditionVar.wait_until(lock, priorityTaskQueue.top().startTime);
                    continue;
                }

                if (priorityTaskQueue.empty())
                {
                    continue;
                }

                // 3. Estrazione del task pronto
                taskToRun = priorityTaskQueue.top();
                priorityTaskQueue.pop();
            }

            // 4. Esecuzione del job al di fuori del blocco di lock
            if (taskToRun.jobCallback)
            {
                taskToRun.jobCallback();
            }
        }
    }

    void submitScheduled(int taskId, Priority taskPriority, std::chrono::steady_clock::time_point startTime, std::function<void()> jobCallback)
    {
        {
            std::lock_guard<std::mutex> lock(schedulerMutex);
            priorityTaskQueue.push(Task{jobCallback, taskPriority, startTime, taskId});
        }
        conditionVar.notify_all();
    }

    void submit(int taskId, Priority taskPriority, std::function<void()> jobCallback)
    {
        submitScheduled(taskId, taskPriority, std::chrono::steady_clock::now(), jobCallback);
    }

private:
    std::priority_queue<Task> priorityTaskQueue;
    std::vector<std::thread> workerThreads;
    std::mutex schedulerMutex;
    std::condition_variable conditionVar; 
    bool isShutdownRequested{false};
};

int main() 
{
    LOG("Core logici disponibili: " << std::thread::hardware_concurrency());
    
    // Inizializzazione dello scheduler con 2 worker thread
    PriorityTaskScheduler taskScheduler(2);

    LOG("--- Invio Task Immediati ---");
    taskScheduler.submit(1, Priority::LOW, []() {
        LOG("[Task 1] Esecuzione LOW priority");
    });

    taskScheduler.submit(2, Priority::CRITICAL, []() {
        LOG("[Task 2] Esecuzione CRITICAL priority (passa davanti)");
    });

    taskScheduler.submit(3, Priority::HIGH, []() {
        LOG("[Task 3] Esecuzione HIGH priority");
    });

    LOG("--- Invio Task Schedulato nel Futuro ---");
    const auto scheduledFutureTime = std::chrono::steady_clock::now() + std::chrono::seconds(2);
    
    taskScheduler.submitScheduled(4, Priority::CRITICAL, scheduledFutureTime, []() {
        LOG("[Task 4] CRITICAL Schedulato (Eseguito dopo 2 secondi)");
    });

    // Attesa per consentire il completamento dei task schedulati
    std::this_thread::sleep_for(std::chrono::seconds(3));

    LOG("--- Fine Test ---");
    return 0;
}