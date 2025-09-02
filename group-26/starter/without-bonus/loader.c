#include "loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;
/*
 * release memory and other cleanups
 */
void loader_cleanup(){

}

// Load and run the ELF executable file
 
void load_and_run_elf(char** exe){
  fd=open(exe[1],O_RDONLY);

  int result = _start();
  printf("User _start return value = %d\n", result);
}

int main(int argc, char** argv) 
{
  if(argc != 2) {
    printf("Usage: %s <ELF Executable> \n",argv[0]);
    exit(1);
  }
  // 1. carry out necessary checks on the input ELF file
  // 2. passing it to the loader for carrying out the loading/execution
  load_and_run_elf(argv);
  // 3. invoke the cleanup routine inside the loader  
  loader_cleanup();
  return 0;
}
