ELF LOADER PROGRAM
This program is a simple ELF (Executable and Linkable Format) loader, designed to load, map, and execute an ELF executable file. It simulates a virtual memory environment, handling page faults dynamically through signal handling. Moreover, it is an upgrade to the Simple Loader created in Assignment 1. Thus, a Smart Loader.

FILES
loader.h: Header file defining the required structures and constants.
loader.c: Contains the main loader functionality and signal handling.
fib.c: A basic program to calculate the Fibonacci series.
sum.c: A given program to calculate the sum of the elements of the array.
makeFile: A given file containing the commands for compiling all the files.

HOW TO RUN?
Run the loader with an ELF file as the argument:
1. make all
2. ./loader <ELF_Executable>

for example
./loader ./fib
./loader ./sum

DESCRIPTION OF MAIN COMPONENTS
File Descriptors and ELF Headers: The program reads and validates ELF and program headers from the specified file.
SIGSEGV Signal Handling: A custom handler sigsegv_handler manages page faults. Each time a page fault occurs, it allocates a page, reads the necessary segment data from the ELF file, and increments counters for monitoring. The handler also tracks internal fragmentation caused by partially filled memory pages.
Counters and Fragmentation Calculation: Tracks page faults, and allocated pages, and calculates internal fragmentation in kilobytes.
Cleanup Function: loader_cleanup() closes file descriptors and frees memory to avoid memory leaks.

OUTPUT
The program outputs the following statistics upon execution:
Return value from the ELF entry point function.
Number of page faults encountered.
Number of pages allocated.
Internal fragmentation (in KB).

CREDITS
Dewang: ReadME, Debugging
Pranshu Prakash: Logic Building, Error handling

Github Link - https://github.com/Pranshu101-hub/OperatingSystems-Assignment
