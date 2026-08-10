/*
 * ============================================================================
 * ESERCIZIO: Parallel Matrix Multiplication (Data Decomposition)
 * ============================================================================
 * SCOPO:
 * Dimostrare la moltiplicazione tra matrici in parallelo mediante la decomposizione 
 * del dominio per righe (Row-wise Partitioning). Il carico di lavoro viene diviso 
 * equamente tra un insieme di worker thread (`std::thread`), ciascuno dei quali 
 * scrive in modo indipendente su porzioni disgiunte della matrice risultato `C`, 
 * evitando la necessita di sincronizzazione tramite mutex.
 *
 * STEP:
 * 1. Definire l'alias `Matrix` basato su `std::vector<std::vector<double>>`.
 * 2. Implementare la funzione worker `multiplyRowRangeTask` per il calcolo nell'intervallo [startRow, endRow).
 * 3. Implementare la funzione orchestratrice `parallelMatrixMultiply` per la partizione equa delle righe e la gestione del ciclo di vita dei thread.
 * 4. Verificare le dimensioni delle matrici di input e stampare il risultato finale formattato.
 *
 * Tags: std::thread, std::cref, std::ref, data-decomposition, matrix-multiplication, parallel-programming
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <string>
#include <vector>
#include <thread>

using Matrix = std::vector<std::vector<double>>;

// Worker thread: Calcola le righe nell'intervallo semi-aperto [startRow, endRow)
void multiplyRowRangeTask(const Matrix& matrixA, const Matrix& matrixB, Matrix& resultMatrix, int startRow, int endRow)
{
    LOG("Thread avviato per le righe: [" << startRow << ", " << endRow - 1 << "]");

    const size_t colsB = matrixB[0].size();
    const size_t colsA = matrixA[0].size();

    for (int i = startRow; i < endRow; ++i)
    {
        for (size_t j = 0; j < colsB; ++j)
        {
            double sum = 0.0;
            for (size_t k = 0; k < colsA; ++k)
            {
                sum += matrixA[i][k] * matrixB[k][j];
            }
            resultMatrix[i][j] = sum;
        }
    }
}

// Orchestratore parallelo
Matrix parallelMatrixMultiply(const Matrix& matrixA, const Matrix& matrixB, int numThreads)
{
    const int rowsA = matrixA.size();
    const int colsB = matrixB[0].size();

    // Pre-allocazione della matrice risultato (RowsA x ColsB)
    Matrix resultMatrix(rowsA, std::vector<double>(colsB, 0.0));

    std::vector<std::thread> workerThreads;
    workerThreads.reserve(numThreads);

    const int baseRows = rowsA / numThreads;
    const int remainingRows = rowsA % numThreads;

    int startRow = 0;
    for (int i = 0; i < numThreads; ++i)
    {
        // Distribuzione equa delle righe rimanenti
        const int currentRows = baseRows + (i < remainingRows ? 1 : 0);
        const int endRow = startRow + currentRows;

        if (startRow < endRow)
        {
            workerThreads.emplace_back(multiplyRowRangeTask, 
                                       std::cref(matrixA), std::cref(matrixB), std::ref(resultMatrix), 
                                       startRow, endRow);
        }

        startRow = endRow;
    }

    // Attesa completamento dei worker thread
    for (auto& threadWorker : workerThreads)
    {
        if (threadWorker.joinable())
        {
            threadWorker.join();
        }
    }

    return resultMatrix;
}

int main() 
{
    constexpr int numThreads = 2;

    LOG("Core logici disponibili: " << std::thread::hardware_concurrency());
    LOG("Thread utilizzati per il test: " << numThreads);

    const Matrix matrixA = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 1, 2, 3},
        {4, 5, 6, 7}
    };

    const Matrix matrixB = {
        {1, 0, 2, 0},
        {0, 3, 0, 4},
        {5, 0, 6, 0},
        {0, 7, 0, 8}
    };

    const int rowsA = matrixA.size();
    const int colsA = matrixA[0].size();
    const int rowsB = matrixB.size();
    const int colsB = matrixB[0].size();

    // Controllo di compatibilita matematica (ColsA == RowsB)
    if (colsA != rowsB)
    {
        LOG("ERRORE: Dimensioni non compatibili per la moltiplicazione! (" 
            << colsA << " != " << rowsB << ")");
        return 1;
    }

    LOG("Dimensioni compatibili. Dimensione matrice C: " << rowsA << "x" << colsB);

    // Calcolo parallelo
    Matrix resultMatrix = parallelMatrixMultiply(matrixA, matrixB, numThreads);

    // Stampa del risultato
    LOG("--- MATRICE RISULTATO C ---");
    for (int i = 0; i < rowsA; ++i)
    {
        std::string rowOutput;
        for (int j = 0; j < colsB; ++j)
        {
            rowOutput += std::to_string(static_cast<int>(resultMatrix[i][j])) + "\t";
        }
        LOG("Riga " << i << ": " << rowOutput);
    } 

    return 0;
}