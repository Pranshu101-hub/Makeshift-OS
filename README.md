# Operating System: From Scratch Implementations

This repository contains a collection of five core Operating System components implemented from scratch in C and C++. These projects were developed as part of the **CSE 231: Operating Systems** course. The primary objective was to interact directly with Unix system calls, handle memory management manually, and build essential system utilities without relying on high-level abstractions or standard libraries where prohibited.

## 🛠 Technologies & Tools
* **Languages:** C, C++ (Limited to C++11 Lambda support for Multithreading) 
* **System APIs:** Unix/Linux System Calls (`mmap`, `fork`, `exec`, `signal`, `pthread`) 
* **Concepts:** ELF Parsing, Demand Paging, Process Scheduling, IPC (Pipes), Concurrency 

---

## 1. SimpleLoader: ELF Executable Loader
**Objective:** To understand the structure of Executable and Linkable Format (ELF) files and memory mapping by building a custom loader in C .

### Implementation Details
* **ELF Parsing:** Implemented a raw parser to read the **ELF Header** and **Program Header Table** of 32-bit executables, extracting entry points and segment offsets .
* **Memory Execution:** Utilized `mmap` to allocate memory for `PT_LOAD` segments and copied segment content from disk to the heap-allocated space .
* **Execution Transfer:** Navigated to the virtual address of the entry point (`e_entry`) and typecasted the address to a function pointer to execute the binary .
* **Constraints:** The loader is a shared library (`lib_simpleloader.so`) compiled with `-nostdlib`, ensuring no reliance on standard system libraries .

---

## 2. SimpleShell: Unix Command Line Interpreter
**Objective:** To master process creation, management, and inter-process communication (IPC) by building a functional Shell .

### Implementation Details
* **REPL Architecture:** Implemented a Read-Eval-Print Loop that accepts user input, parses commands, and executes them in an infinite loop .
* **Process Management:** Utilized `fork()` to create child processes and the `exec` family of functions to run system commands (e.g., `ls`, `grep`, `wc`) .
* **Piping Support:** Implemented IPC using pipes (`|`), allowing the output of one command to serve as the input for another (e.g., `cat fib.c | wc -l`) .
* **History & Analytics:** Maintained a command history that tracks execution time and process PIDs, displayed upon shell termination .

---

## 3. SimpleScheduler: Round-Robin Process Scheduler
**Objective:** To simulate OS process scheduling on limited CPU resources using signals and state management .

### Implementation Details
* **Integration with Shell:** Extended the *SimpleShell* to allow users to `submit` jobs. A daemon scheduler process manages these jobs based on user-defined `NCPU` (available cores) and `TSLICE` (time quantum) .
* **Round-Robin Logic:** Implemented a Ready Queue where the scheduler signals `NCPU` processes to run, waits for the `TSLICE` to expire, pauses them (context switch), and rotates them to the back of the queue .
* **Signal Handling:** Used signals to start, stop, and resume user processes (executables hooked with a `dummy_main.h` header) .
* **Metrics:** Upon termination, the system reports the name, PID, completion time, and wait time for all submitted jobs .

---

## 4. SimpleSmartLoader: Demand Paging Implementation
**Objective:** To optimize the *SimpleLoader* by implementing "lazy loading" and handling page faults dynamically, mimicking modern OS memory management .

### Implementation Details
* **Demand Paging:** Unlike the standard loader, this version does *not* load segments upfront. It immediately attempts to execute the entry point, intentionally triggering a Segmentation Fault due to unallocated pages .
* **Signal Handling as Page Faults:** Intercepted `SIGSEGV` signals. If the fault address belongs to a valid segment, the handler treats it as a page fault rather than a crash .
* **Page-by-Page Allocation:** Dynamically allocated memory using `mmap` in 4KB page chunks only when accessed, resolving internal fragmentation issues .
* **Memory Statistics:** The loader reports total page faults, page allocations, and internal fragmentation statistics after execution .

---

## 5. SimpleMultithreader: Parallelization Library
**Objective:** To abstract low-level Pthread management into a high-level C++11 interface for loop parallelization .

### Implementation Details
* **Lambda Support:** Created a header-only library (`simple-multithreader.h`) accepting C++11 lambda functions for parallel execution .
* **`parallel_for` API:** Implemented methods to parallelize 1D and 2D loops by dividing the iteration space across `numThreads` specified by the user .
* **Low-Level Threading:** Backend strictly uses `pthread` (disallowing `std::thread`), manually managing thread creation, joining, and argument passing without a thread pool .
* **Performance Measurement:** Automatically calculates and reports execution time for parallelized blocks .

---

## Build Instructions
Each project contains a dedicated `Makefile`. To build the entire suite, navigate to the respective directories and run:

```bash
make
