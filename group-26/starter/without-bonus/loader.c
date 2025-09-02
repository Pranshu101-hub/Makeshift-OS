#include "loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;
/*
 * release memory and other cleanups
 */
void loader_cleanup(){
  if(fd >= 0){
    close(fd);
  }
  if(ehdr){
    free(ehdr);
    ehdr = NULL;
  }
  if(phdr){
    free(phdr);
    phdr = NULL;
  }

}

// Load and run the ELF executable file
 
void load_and_run_elf(char** exe){
  fd=open(exe[1],O_RDONLY);
  if(fd < 0){
    perror("Failed to open file");
    exit(1);
  }
  // 1. Load entire binary content into the memory from the ELF file.
  ehdr = (Elf32_Ehdr*)malloc(sizeof(Elf32_Ehdr));
  if(!ehdr){
    perror("Memory allocation failed for ehdr");
    loader_cleanup();
    exit(1);
  }
  ssize_t read_bytes = read(fd, ehdr, sizeof(Elf32_Ehdr));
  if (read_bytes< 0) {
    perror("Failed to read ELF header");
    loader_cleanup();
    exit(1);
} else if (read_bytes != sizeof(Elf32_Ehdr)) {
    perror("File too short");
    loader_cleanup();
    exit(1);
}
  phdr = (Elf32_Phdr*)malloc(ehdr->e_phentsize * ehdr->e_phnum); // Allocate memory for program headers = entry size * no. of entries
  if(!phdr){
    perror("Memory allocation failed for phdr");
    loader_cleanup();
    exit(1);
  }
  if(lseek(fd,ehdr->e_phoff,SEEK_SET) < 0){
    perror("Failed to seek program headers");
    loader_cleanup();
    exit(1);
  }
  ssize_t phdr_read_bytes = read(fd, phdr, ehdr->e_phentsize * ehdr->e_phnum);
  if (phdr_read_bytes < 0) {
    perror("Failed to read phdr");
    loader_cleanup();
    exit(1);
  } 
  else if (phdr_read_bytes != sizeof(Elf32_Phdr)) {
    perror("Incomplete read of program header");
    loader_cleanup();
    exit(1);
  }

  // 2. Iterate through the PHDR table and find the section of PT_LOAD 
  //    type that contains the address of the entrypoint method in fib.c
  Elf32_Phdr* phdr_load = NULL;
  for(int i=0;i<ehdr->e_phnum;i++){
    Elf32_Phdr* curr = &phdr[i];
    if(curr->p_type == PT_LOAD){
      if(curr->p_vaddr <= ehdr->e_entry && ehdr->e_entry <= curr->p_vaddr + curr->p_memsz){
        phdr_load = curr;
        break;
      }
    }
  }
  if(!phdr_load){
    fprintf(stderr,"Load Segment not found\n");
    loader_cleanup();
    exit(1);
  }
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
