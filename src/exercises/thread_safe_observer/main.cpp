/*
 * ============================================================================
 * ESERCIZIO: Thread-Safe Observer Pattern using std::weak_ptr and std::mutex
 * ============================================================================
 * SCOPO:
 * Dimostrare l'implementazione thread-safe del pattern Observer. L'uso di `std::weak_ptr` 
 * previene i cicli di riferimento (memory leak) e la pendendenza da osservatori deallocati (dangling pointers). 
 * Inoltre, l'acquisizione temporanea con `.lock()` combinata con una copia locale dei puntatori 
 * rilascia il mutex PRIMA dell'invocazione di `onEvent`, evitando potenziali Deadlock.
 *
 * STEP:
 * 1. Definire le interfacce `Observer` e `ConcreteObserver`.
 * 2. Implementare la classe `Subject` memorizzando gli osservatori come `std::weak_ptr<Observer>`.
 * 3. Implementare `unsubscribeObserver` usando l'idioma Erase-Remove (`std::remove_if`).
 * 4. In `notifyObservers`, convertire temporaneamente i `weak_ptr` validi in `shared_ptr`, rilasciare il mutex ed invocare le callback.
 * 5. Testare l'accesso concorrente avviando un thread notificatore (`publisherThread`) ed uno di modifica (`modifierThread`).
 *
 * Tags: std::thread, observer-pattern, std-weak-ptr, std-shared-ptr, thread-safety, erase-remove-idiom, deadlock-prevention
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <chrono>
#include <thread>

class Observer 
{
public:
    virtual ~Observer() = default;
    virtual void onEvent(const std::string& messageFromSubject) = 0;
};

class ConcreteObserver : public Observer 
{
public:
    explicit ConcreteObserver(int observerId) : observerId(observerId) {}

    void onEvent(const std::string& messageFromSubject) override 
    {
        LOG("[Observer " << observerId << "] Ricevuto messaggio: " << messageFromSubject);
    }

private:
    int observerId;
};

class Subject 
{
public:
    Subject() = default;

    void subscribeObserver(const std::shared_ptr<Observer>& observerInstance)
    {
        if (!observerInstance) 
        {
            return;
        }

        std::lock_guard<std::mutex> lockGuard(subjectMutex);
        observerWeakList.push_back(observerInstance);
    }

    void unsubscribeObserver(const std::shared_ptr<Observer>& observerInstance)
    {
        if (!observerInstance) 
        {
            return;
        }

        std::lock_guard<std::mutex> lockGuard(subjectMutex);
        
        // Idioma Erase-Remove per pulire l'osservatore e rimuovere eventuali weak_ptr scaduti
        observerWeakList.erase(
            std::remove_if(observerWeakList.begin(), observerWeakList.end(),
                [&observerInstance](const std::weak_ptr<Observer>& weakObserverPointer) {
                    auto sharedObserverPointer = weakObserverPointer.lock();
                    return !sharedObserverPointer || sharedObserverPointer == observerInstance;
                }),
            observerWeakList.end()
        );
    }

    void notifyObservers(const std::string& messageToPublish)
    {
        std::vector<std::shared_ptr<Observer>> activeObserversList;

        // Sezione critica breve: isola la conversione dei weak_ptr senza bloccare le callback estese
        {
            std::lock_guard<std::mutex> lockGuard(subjectMutex);
            for (const auto& weakObserverPointer : observerWeakList)
            {
                if (auto sharedObserverPointer = weakObserverPointer.lock()) 
                {
                    activeObserversList.push_back(sharedObserverPointer);
                } 
            }
        }

        // Notifica eseguita fuori dalla sezione critica (senza mutex acquisito) per prevenire deadlock
        for (const auto& activeObserver : activeObserversList)
        {
            activeObserver->onEvent(messageToPublish);
        }
    }

private: 
    std::vector<std::weak_ptr<Observer>> observerWeakList;
    mutable std::mutex subjectMutex;
};

int main() 
{
    LOG("=== Esercizio 12: Thread-Safe Observer Pattern ===");
    
    Subject notificationSubject;

    // Creazione di due osservatori persistenti
    auto primaryObserver = std::make_shared<ConcreteObserver>(1);
    auto secondaryObserver = std::make_shared<ConcreteObserver>(2);

    notificationSubject.subscribeObserver(primaryObserver);
    notificationSubject.subscribeObserver(secondaryObserver);

    // Thread 1: Notifica periodica in ciclo
    std::thread publisherThread([&notificationSubject]() {
        for (int messageIndex = 1; messageIndex <= 3; ++messageIndex) 
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            notificationSubject.notifyObservers("Messaggio #" + std::to_string(messageIndex));
        }
    });

    // Thread 2: Aggiunta e rimozione dinamica di un terzo osservatore in concorrenza
    std::thread modifierThread([&notificationSubject]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        
        auto temporaryObserver = std::make_shared<ConcreteObserver>(99);
        notificationSubject.subscribeObserver(temporaryObserver);
        LOG("[Modifier Thread] Iscritto Observer 99");

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        notificationSubject.unsubscribeObserver(temporaryObserver);
        LOG("[Modifier Thread] Disiscritto Observer 99");
    });

    if (publisherThread.joinable()) 
    {
        publisherThread.join();
    }
    
    if (modifierThread.joinable()) 
    {
        modifierThread.join();
    }

    LOG("--- Test completato con successo ---");
    return 0;
}