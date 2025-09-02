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
  // 3. Allocate memory of the size "p_memsz" using mmap function 
  //    and then copy the segment content
  void *mem = mmap(NULL,phdr_load->p_memsz, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS| MAP_PRIVATE, 0,0);
  if(mem == MAP_FAILED){
    perror("Memory mapping failed");
    loader_cleanup();
    exit(1);
  }
  
  if(lseek(fd,phdr_load->p_offset, SEEK_SET) < 0){
    perror("Failed to seek segment");
    loader_cleanup();
    exit(1);
  }
  ssize_t segment_read_bytes = read(fd, mem, phdr_load->p_filesz);
  if (segment_read_bytes < 0) {
    perror("Failed to read segment data");
    loader_cleanup();
    exit(1);

} else if (segment_read_bytes != (ssize_t)phdr_load->p_filesz) {
    perror("Error: Incomplete segment read ");
    loader_cleanup();
    exit(1);
}
  // 4. Navigate to the entrypoint address into the segment loaded in the memory in above step
  int (_start)() = (int()()) (mem + (ehdr->e_entry - phdr_load->p_vaddr));
  // 5. Typecast the address to that of a function pointer matching the "_start" method in fib.c.
  // 6. Call the "_start" method and print the value returned from the "_start"
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
