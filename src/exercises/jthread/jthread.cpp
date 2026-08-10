/*
 * ============================================================================
 * ESERCIZIO: Thread Creation Methods (std::jthread - C++20)
 * ============================================================================
 * SCOPO:
 * Dimostrare le diverse modalità per istanziare ed eseguire task tramite 
 * std::jthread (C++20): funzioni libere con parametri, espressioni Lambda, 
 * metodi di istanza, metodi statici e Functor (operator() overloading).
 *
 * STEP:
 * 1. Avviare t1 passando una funzione libera e un parametro per valore.
 * 2. Avviare t2 passando una Lambda function inline.
 * 3. Avviare t3 passando un metodo membro non statico e il puntatore all'oggetto (&Test::stampa1, &test).
 * 4. Avviare t4 passando un metodo membro statico di una classe.
 * 5. Avviare t5 passando un Functor e i relativi argomenti.
 * 6. Sfruttare la RAII di std::jthread per la sincronizzazione (join) automatica al distruggersi dello scope.
 *
 * Tags: std::jthread, std::thread::hardware_concurrency
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <string>
#include <thread>

void stampa(std::string stringa)
{
    LOG("\033[1;31m" << stringa << "\033[0m");
}

class Test
{
public:
    Test() = default;

    void stampa1() 
    {
        LOG("\033[1;34mHello World in Blue!\033[0m");
    }

    static void stampa2()
    {
        LOG("\033[1;33mHello World in Yellow!\033[0m");
    }
};

struct Functor 
{
    void operator()(int n) 
    {
        for (int i = 0; i < n; ++i) 
        {
            LOG("Functor running: " << i);
        }
    }
};

int main() 
{
    LOG("=== Thread Creation Methods (C++20 std::jthread) ===");
    LOG("Core logici disponibili: " << std::thread::hardware_concurrency());
    
    // 1. Funzione libera con parametro
    std::string stringa = "Hello World in Red!";
    std::jthread t1(stampa, stringa);

    // 2. Lambda function con parametro
    std::jthread t2([](std::string str) {
        LOG("\033[1;32m" << str << "\033[0m");
    }, "Hello World in Green!");

    // 3. Metodo di istanza (richiede indirizzo del metodo e dell'oggetto)
    Test test;
    std::jthread t3(&Test::stampa1, &test);

    // 4. Metodo statico di classe
    std::jthread t4(&Test::stampa2);

    // 5. Functor (Callable Object)
    std::jthread t5(Functor{}, 3);

    // std::jthread effettua automaticamente il join() alla distruzione (RAII)
    return 0;
}