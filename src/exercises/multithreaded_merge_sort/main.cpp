/*
 * ============================================================================
 * ESERCIZIO: Parallel Merge Sort using Divide and Conquer
 * ============================================================================
 * SCOPO:
 * Dimostrare l'implementazione dell'algoritmo Divide et Impera Parallel Merge 
 * Sort sfruttando std::thread per il partizionamento ricorsivo dei dati e 
 * std::inplace_merge per la fase di combinazione ordinata.
 *
 * STEP:
 * 1. Calcolare la dimensione della sequenza tramite iteratori e verificare le condizioni di stop (soglia minima o profondita max).
 * 2. Dividere la sequenza a meta, delegando l'elaborazione del ramo sinistro a un nuovo std::thread.
 * 3. Processare ricorsivamente il ramo destro sul thread corrente.
 * 4. Effettuare la sincronizzazione via .join() e ricombinare le due meta ordinate tramite std::inplace_merge.
 *
 * Tags: std::thread, std::thread::joinable, std::thread::join, std::inplace_merge, std::sort, divide-and-conquer
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <thread>

template <typename Iterator>
void parallelMergeSort(Iterator begin, Iterator end, int currentDepth = 0)
{
    const auto elementCount = std::distance(begin, end);

    constexpr std::ptrdiff_t minThreshold = 4;
    constexpr int maxDepth = 4;

    // Condizioni di stop: dimensione contenuta o limite di profondita raggiunto
    if (elementCount <= minThreshold || currentDepth > maxDepth)
    {
        std::sort(begin, end);
        return;
    }

    const auto middle = begin + elementCount / 2;

    // Fork: Avviamo l'elaborazione ricorsiva del ramo sinistro su un thread dedicato
    std::thread leftWorkerThread(parallelMergeSort<Iterator>, begin, middle, currentDepth + 1);

    // Il thread corrente gestisce il ramo destro
    parallelMergeSort(middle, end, currentDepth + 1);

    // Join: Attesa del completamento del ramo sinistro
    if (leftWorkerThread.joinable())
    {
        leftWorkerThread.join();
    }

    // Merge in-place delle due meta gia ordinate
    std::inplace_merge(begin, middle, end);
}

int main() 
{
    LOG("Core logici disponibili: " << std::thread::hardware_concurrency());

    std::vector<int> unorderedData = 
    { 38, 27, 43, 3, 9, 82, 10, 19, 50, 12, 7, 64, 25, 31, 88, 5 };

    std::string initialOutput;
    for (const auto value : unorderedData)
    {
        initialOutput += std::to_string(value) + " ";
    }
    LOG("Vettore NON ordinato: " << initialOutput);

    parallelMergeSort(unorderedData.begin(), unorderedData.end());

    std::string sortedOutput;
    for (const auto value : unorderedData)
    {
        sortedOutput += std::to_string(value) + " ";
    }
    LOG("Vettore ORDINATO:     " << sortedOutput);

    return 0;
}