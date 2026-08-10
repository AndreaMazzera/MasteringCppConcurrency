/*
 * ============================================================================
 * ESERCIZIO: Multi-Stage Data Pipeline with Thread-Safe Queue
 * ============================================================================
 * SCOPO:
 * Dimostrare l'architettura di elaborazione in parallelo a stadi (Pipeline Architecture) 
 * sfruttando una coda bloccante thread-safe (`ThreadSafeQueue`) basata su `std::queue`, 
 * `std::mutex` e `std::condition_variable`. Illustra la propagazione ordinata 
 * del segnale di arresto a cascata (Cascading Shutdown) dal primo all'ultimo stadio.
 *
 * STEP:
 * 1. Implementare la classe template `ThreadSafeQueue` con supporto a `push`, `pop` bloccante e `shutdown`.
 * 2. Implementare la task dello Stage 1 (`fetchStageWorker`): genera i pacchetti grezzi e li inserisce nella prima coda.
 * 3. Implementare la task dello Stage 2 (`decodeStageWorker`): estrae i pacchetti, trasforma i dati in maiuscolo e inserisce i frame nella seconda coda.
 * 4. Implementare la task dello Stage 3 (`processStageWorker`): consuma i dati finali e li registra nel log.
 * 5. Avviare i tre thread nel `main` coordinando la pipeline e attendere il completamento tramite `.join()`.
 *
 * Tags: std::thread, std::mutex, std::condition_variable, std::queue, pipeline-pattern, thread-safe-queue, cascading-shutdown
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <algorithm>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <thread>

struct RawPacket 
{
    int id;
    std::string rawData;
};

struct DecodedFrame 
{
    int id;
    std::string decodedData;
};

struct ProcessedResult 
{
    int id;
    std::string finalOutput;
};

template<typename T>
class ThreadSafeQueue 
{
public:
    ThreadSafeQueue() = default;

    void push(T value)
    {
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            dataQueue.push(std::move(value));
        }
        conditionVar.notify_one();
    }

    bool pop(T& value)
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        conditionVar.wait(lock, [this] { return !dataQueue.empty() || isShutdownRequested; });

        if (dataQueue.empty() && isShutdownRequested)
        {
            return false;
        }

        value = std::move(dataQueue.front());
        dataQueue.pop();
        return true;
    }

    void shutdown()
    {
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            isShutdownRequested = true;
        }
        conditionVar.notify_all(); 
    }

private:
    std::queue<T> dataQueue;
    std::mutex queueMutex;
    std::condition_variable conditionVar;
    bool isShutdownRequested{false};
};

void fetchStageWorker(ThreadSafeQueue<RawPacket>& outputQueue, int packetCount) 
{
    for (int i = 1; i <= packetCount; ++i) 
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        
        RawPacket packet{i, "raw_payload_" + std::to_string(i)};
        LOG("[Stage 1 - Fetch] Generato pacchetto #" << i);
        outputQueue.push(packet);
    }
    
    // Propagazione dello shutdown allo Stage 2
    outputQueue.shutdown();
}

void decodeStageWorker(ThreadSafeQueue<RawPacket>& inputQueue, ThreadSafeQueue<DecodedFrame>& outputQueue) 
{
    RawPacket rawPacket;
    
    while (inputQueue.pop(rawPacket)) 
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
        
        std::string decodedText = rawPacket.rawData;
        std::transform(decodedText.begin(), decodedText.end(), decodedText.begin(), ::toupper);

        LOG("[Stage 2 - Decode] Decodificato pacchetto #" << rawPacket.id);
        outputQueue.push(DecodedFrame{rawPacket.id, decodedText});
    }

    // Propagazione dello shutdown allo Stage 3
    outputQueue.shutdown();
}

void processStageWorker(ThreadSafeQueue<DecodedFrame>& inputQueue) 
{
    DecodedFrame decodedFrame;
    
    while (inputQueue.pop(decodedFrame)) 
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
        
        LOG("[Stage 3 - Process] SALVATO RISULTATO #" << decodedFrame.id << " -> " << decodedFrame.decodedData);
    }

    LOG("[Stage 3 - Process] Pipeline completata con successo!");
}

int main() 
{
    LOG("Core logici disponibili: " << std::thread::hardware_concurrency());
    
    ThreadSafeQueue<RawPacket> rawQueue; 
    ThreadSafeQueue<DecodedFrame> decodedQueue;

    std::thread fetchThread(fetchStageWorker, std::ref(rawQueue), 5);
    std::thread decodeThread(decodeStageWorker, std::ref(rawQueue), std::ref(decodedQueue));
    std::thread processThread(processStageWorker, std::ref(decodedQueue));

    if (fetchThread.joinable())
    {
        fetchThread.join();
    }
    
    if (decodeThread.joinable())
    {
        decodeThread.join();
    }
    
    if (processThread.joinable())
    {
        processThread.join();
    }

    return 0;
}