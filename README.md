# ⚡ Modern C++ Multithreading & Concurrency Workspace

Repository dedicato allo studio, alla progettazione e all'implementazione di pattern avanzati di **concorrenza e multithreading in C++ moderno (C++17 / C++20)**. 

Il progetto combina esercizi di sincronizzazione ad alte prestazioni, sistemi concorrenti reali e progetti architetturali orientati al massimo throughput su CPU multicore.

## 📂 Struttura del Repository

La struttura rispetta le best-practice per progetti C++ modulari e scalabili:

```text
Multithreading/
+-- CMakeLists.txt          # Root CMake build system
+-- LICENSE                 # Licenza Open Source (MIT)
+-- .gitignore              # Configurazione Git per artefatti e IDE
|
+-- doc/                    # Documentazione interattiva e note teoriche
|   +-- main.html           # Viewer web dinamico per la documentazione
|   +-- fileList.json       # Indice dei file Markdown renderizzati
|   +-- *.md                # Appunti approfonditi
|
+-- src/                    # Codice Sorgente
    +-- CMakeLists.txt      # Subdirectory CMake principale
    +-- commons/            # Logger e utility condivise (common_utils)
    +-- exercises/          # Esercizi pratici di sincronizzazione e lock-patterns
    +-- projects/           # Progetti complessi e sistemi concorrenti
```

## 🛠️ Requisiti e Compilazione

### Requisiti
* **Compilatore C++:** GCC (v11+), Clang (v13+), o MSVC con supporto a **C++20**
* **Build System:** CMake 3.16+
* **Toolchain consigliata:** MinGW-w64 (UCRT64) su Windows, Native Make/Ninja su Linux & macOS

### Procedura di Build

1. Clona il repository:
   git clone [https://github.com/AndreaMazzera/MasteringCppConcurrency.git](https://github.com/AndreaMazzera/MasteringCppConcurrency.git)
   cd Multithreading

2. Configura e compila tramite CMake:
   cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
   cmake --build build --parallel

3. Installazione locale dei binari (opzionale):
   cmake --install build --prefix install
   (Gli eseguibili compilati verranno esportati nella cartella install/bin/)


## 🚀 Esercizi Implementati (src/exercises/)

Tutti gli esercizi integrano una gestione thread-safe accurata e metriche trasparenti:

- **Cross-Platform Chat Server:** POSIX / Winsock2 (ws2_32), std::mutex, Thread Detached.
- **Parallel File Downloader:** Partitioning O(1), Parallel File I/O, std::atomic<size_t>, Progress Monitor.
- **Concert Ticket Booking System:** Fine-Grained Locking, Lock-Striping, std::shared_mutex, std::unique_lock.
- **Parallel Web Crawler:** Task Distribution, Deadlock Prevention, std::condition_variable, std::unordered_set.


## 🔮 TODO: Progetti Avanzati (src/projects/)

Progetti architetturali ad alto throughput pensati per mettere alla prova la gestione della concorrenza su larga scala:

1. **Adaptive Thread Pool con Work-Stealing**
   - Coda di task generica basata su std::packaged_task e std::future.
   - Algoritmo di Work-Stealing tra code locali per abbattere la contesa sui lock.
   - Gestione del ciclo di vita con std::jthread (C++20) e cancellazione cooperativa.

2. **In-Memory Key-Value Store Concorrente**
   - Concurrent Hash Map basata su Lock Striping.
   - Letture parallele non bloccanti tramite std::shared_mutex (C++17).
   - Thread di Garbage Collection in background per l'eliminazione asincrona dei TTL.

3. **Server HTTP Asincrono con I/O Multiplexing**
   - Event Loop basato sulle primitive del kernel (epoll / kqueue).
   - Architettura Reactor/Proactor disaccoppiata con Worker Pool.
   - Tracciamento delle metriche con std::atomic e memory orderings espliciti.

4. **Motore di Simulazione / Ray Tracer Parallelo**
   - Partizionamento delle immagini in chunk per massimizzare la Data Parallelism.
   - Sincronizzazione di fase per frame tramite std::barrier / std::latch (C++20).
   - Ottimizzazione della Cache Locality ed eliminazione del False Sharing (alignas).

5. **Multithreaded Cross-Platform Chat Server (POSIX & Winsock2)**
   - Listener principale per l'accept delle connessioni e dispatching client su thread detached.
   - Registro delle sessioni e broadcast dei messaggi thread-safe gestiti tramite std::mutex.
   - Astrazione delle chiamate socket per la cross-compilazione tra POSIX e Windows.

6. **Multithreaded Parallel File Downloader**
   - Partizionamento di file di grandi dimensioni in blocchi di byte non sovrapposti.
   - Pre-allocazione rapida su disco con std::filesystem::resize_file e posizionamento con seekp().
   - Thread di monitoraggio dedicato con tracciamento atomico dell'avanzamento tramite std::atomic<size_t>.

7. **Concert Ticket Booking System (Fine-Grained Concurrency)**
   - Lock-Striping a livello di singolo posto con std::shared_mutex dedicato per risorsa.
   - Accesso O(1) ed eliminazione della contention tra transazioni su posti differenti.
   - Operazioni di lettura simultanee (std::shared_lock) ed esclusive di acquisto/prenotazione (std::unique_lock).

8. **Parallel Web Crawler using Threads**
   - Distribuzione del lavoro tramite coda di task thread-safe condivisa e tracciamento URL visitati con std::mutex.
   - Sincronizzazione e prevenzione dei deadlock tramite std::condition_variable e contatore di worker attivi.
   - Gestione della terminazione deterministica al raggiungimento della quota di pagine.


## 📚 Documentazione Interattiva (doc/)

Il repository include una dashboard web di documentazione integrata.
Per visualizzarla:
- Apri il file doc/main.html in qualsiasi browser moderno.
- Il viewer renderizza dinamicamente i file Markdown degli appunti (multithreading.md, smart_pointers.md, references.md) mantenendo la formattazione avanzata.


## 📜 Licenza

Rilasciato sotto licenza **MIT**. Consulta il file LICENSE per ulteriori dettagli.