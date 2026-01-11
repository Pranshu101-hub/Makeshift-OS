### ***Simple Loader Implementation***



* The program loads a simple ELF (Executable and Linkable Format) file.
* First, it reads the ELF header (Elf32\_Ehdr) to fetch key information like entry point and program headers.
* Memory is allocated for the program headers (Elf32\_Phdr) and they are read from the file.
* Each header is checked to identify which segment contains executable data.
* If a segment is valid, it is mapped into memory using mmap().
* The segment’s contents are copied into memory, and the entry point address is set up as a function pointer.
* Control is transferred to this entry point, and the program begins execution.
* After execution, the program prints the final status.
* In cleanup:

&nbsp;	1) File descriptors are closed.

&nbsp;	2) Allocated memory is freed.

* If errors occur (e.g., file corruption, memory allocation failure), the error handler manages them.





### ***Contributions***

**Dewang**: 

Implemented ELF file loading, including reading the ELF and program headers. 

Also ensured the correct loadable segment was identified before execution.



**Pranshu**: 

Handled mapping the segment into memory and setting up the entry point. 

Also managed program execution, cleanup, and error handling.





***GitHub Link***

---

**Link - https://github.com/Pranshu101-hub/OperatingSystems-Assignment**



