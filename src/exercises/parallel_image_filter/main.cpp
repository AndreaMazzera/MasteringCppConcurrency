/*
 * ============================================================================
 * ESERCIZIO: Image Blur Filter - Sequential vs Parallel Benchmark
 * ============================================================================
 * SCOPO:
 * Dimostrare l'applicazione del parallelismo a dominii di dati 2D (Data Parallelism) 
 * mediante il partizionamento per righe (Horizontal Slicing) di una matrice di 
 * pixel. Mette a confronto le prestazioni di un filtro di sfocatura (Box Blur 3x3) 
 * eseguito in modalità sequenziale rispetto a una versione multi-threaded con std::thread.
 *
 * STEP:
 * 1. Definire le strutture dati `Pixel` e `Image` per rappresentare un'immagine RGB.
 * 2. Generare un'immagine di test con valori casuali di colore mediante `std::mt19937`.
 * 3. Implementare il filtro Box Blur sequenziale che calcola la media dei 9 pixel adiacenti.
 * 4. Implementare la versione parallela dividendo l'altezza dell'immagine in strisce (fette) distribuite tra i thread.
 * 5. Misurare i tempi di esecuzione con `std::chrono` e calcolare lo Speedup finale.
 *
 * Tags: std::thread, std::cref, std::ref, data-parallelism, image-processing, benchmark, speedup
 * ============================================================================
 */

#include "logger.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <thread>

struct Pixel 
{
    unsigned char r{0};
    unsigned char g{0};
    unsigned char b{0};
};

using Image = std::vector<Pixel>;

Image createTestImage(int width, int height) 
{
    Image image(width * height);
    std::mt19937 rngEngine(42); 
    std::uniform_int_distribution<int> colorDist(0, 255);

    for (int i = 0; i < width * height; ++i) 
    {
        image[i] = { static_cast<unsigned char>(colorDist(rngEngine)),
                     static_cast<unsigned char>(colorDist(rngEngine)),
                     static_cast<unsigned char>(colorDist(rngEngine)) };
    }
    return image;
}

// 1. Algoritmo Sequenziale
void applySequentialBlur(int width, int height, const Image& inputImage, Image& outputImage)
{
    for (int y = 1; y < height - 1; ++y)
    {
        for (int x = 1; x < width - 1; ++x)
        {
            int sumRed = 0;
            int sumGreen = 0;
            int sumBlue = 0;

            for (int dy = -1; dy <= 1; ++dy)
            {
                for (int dx = -1; dx <= 1; ++dx)
                {
                    const int neighborIdx = (y + dy) * width + (x + dx);
                    sumRed += inputImage[neighborIdx].r;
                    sumGreen += inputImage[neighborIdx].g;
                    sumBlue += inputImage[neighborIdx].b;
                }
            }

            const int currentIdx = y * width + x;
            outputImage[currentIdx].r = static_cast<unsigned char>(sumRed / 9);
            outputImage[currentIdx].g = static_cast<unsigned char>(sumGreen / 9);
            outputImage[currentIdx].b = static_cast<unsigned char>(sumBlue / 9);
        }
    }
}

// Funzione worker per la porzione di righe assegnata al singolo thread
void blurWorkerTask(int width, int startY, int endY, const Image& inputImage, Image& outputImage)
{
    for (int y = startY; y < endY; ++y)
    {
        for (int x = 1; x < width - 1; ++x)
        {
            int sumRed = 0;
            int sumGreen = 0;
            int sumBlue = 0;

            for (int dy = -1; dy <= 1; ++dy)
            {
                for (int dx = -1; dx <= 1; ++dx)
                {
                    const int neighborIdx = (y + dy) * width + (x + dx);
                    sumRed += inputImage[neighborIdx].r;
                    sumGreen += inputImage[neighborIdx].g;
                    sumBlue += inputImage[neighborIdx].b;
                }
            }

            const int currentIdx = y * width + x;
            outputImage[currentIdx].r = static_cast<unsigned char>(sumRed / 9);
            outputImage[currentIdx].g = static_cast<unsigned char>(sumGreen / 9);
            outputImage[currentIdx].b = static_cast<unsigned char>(sumBlue / 9);
        }
    }
}

// 2. Algoritmo Parallelo
void applyParallelBlur(int width, int height, const Image& inputImage, Image& outputImage, int numThreads)
{
    std::vector<std::thread> workerThreads;
    workerThreads.reserve(numThreads);

    const int usableHeight = height - 2;
    const int rowsPerThread = usableHeight / numThreads;

    for (int i = 0; i < numThreads; ++i)
    {
        const int startY = 1 + i * rowsPerThread;
        const int endY = (i == numThreads - 1) ? (height - 1) : (startY + rowsPerThread);

        workerThreads.emplace_back(blurWorkerTask, width, startY, endY, std::cref(inputImage), std::ref(outputImage));
    }

    for (auto& threadWorker : workerThreads)
    {
        if (threadWorker.joinable())
        {
            threadWorker.join();
        }
    }
}

int main() 
{
    constexpr int numThreads = 6;
    constexpr int imageWidth = 4000;
    constexpr int imageHeight = 4000;

    LOG("Core logici disponibili: " << std::thread::hardware_concurrency());
    LOG("Thread utilizzati per il test: " << numThreads);

    LOG("Inizializzazione immagine " << imageWidth << "x" << imageHeight << "...");
    Image inputImage = createTestImage(imageWidth, imageHeight);
    Image sequentialOutput(imageWidth * imageHeight);
    Image parallelOutput(imageWidth * imageHeight);

    // Single Thread
    const auto startSingleThread = std::chrono::high_resolution_clock::now();
    applySequentialBlur(imageWidth, imageHeight, inputImage, sequentialOutput);
    const auto endSingleThread = std::chrono::high_resolution_clock::now();
    const auto singleThreadDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endSingleThread - startSingleThread);
    LOG("=== Tempo Sequenziale: " << singleThreadDuration.count() << " ms ===");

    // Multi Thread
    const auto startMultiThread = std::chrono::high_resolution_clock::now();
    applyParallelBlur(imageWidth, imageHeight, inputImage, parallelOutput, numThreads);
    const auto endMultiThread = std::chrono::high_resolution_clock::now();
    const auto multiThreadDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endMultiThread - startMultiThread);
    LOG("=== Tempo Parallelo: " << multiThreadDuration.count() << " ms ===");

    // Calcolo dello Speedup
    const double speedup = static_cast<double>(singleThreadDuration.count()) / multiThreadDuration.count();
    LOG("Speedup raggiunto: " << speedup << "x");

    return 0;
}