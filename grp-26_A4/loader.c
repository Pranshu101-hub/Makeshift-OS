#include "loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd = -1;

// global variables
int pageFault_counter = 0;       
int pageAlloc_counter = 0;       
int fragementation = 0;          
long page_size = 4096;           
int array_size = 0;              
int *segment_index;              
int segments_read = 0;

void loader_cleanup() {
    if (fd >= 0) {
        close(fd);
    }
    free(ehdr);
    free(phdr);
}

// checks if the segment has already been loaded
bool segment_check(int segment) {
    int count = 0;
    for(int i = 0; i < array_size; i++) if (segment_index[i] == segment) count++;
    return count == 1;
}

// SIGSEGV handler handles the page faults
void sigsegv_handler(int signum, siginfo_t *info, void *context) {
    pageFault_counter++;
    unsigned long fault_addr = (unsigned long)info->si_addr;

    // find which segment caused the fault
    for (int i = 0; i < ehdr->e_phnum; i++) {
        Elf32_Phdr *segment = &phdr[i];

        unsigned long seg_start = segment->p_vaddr;
        unsigned long seg_end = segment->p_vaddr + segment->p_memsz;

        if (fault_addr >= seg_start && fault_addr < seg_end) {

            // align fault address to page boundary
            unsigned long page_base = fault_addr & ~(page_size - 1);

            // compute file offset for this page
            unsigned long file_offset = segment->p_offset + (page_base - segment->p_vaddr);

            // determine how many bytes to read
            size_t bytes_to_read = page_size;
            if ((page_base + page_size) > seg_end) {
                bytes_to_read = seg_end - page_base;
            }

            // mmap a single page lazily
            void *mapped_page = mmap((void *)page_base, page_size,
                                     PROT_READ | PROT_WRITE | PROT_EXEC,
                                     MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS,
                                     -1, 0);
            if (mapped_page == MAP_FAILED) {
                perror("mmap failed");
                exit(1);
            }

            // copy bytes from file into the mapped page
            lseek(fd, file_offset, SEEK_SET);
            int read_bytes = read(fd, mapped_page, bytes_to_read);
            if (read_bytes < 0) {
                perror("read failed in sigsegv_handler");
                exit(1);
            }

            // update stats
            pageAlloc_counter++;
            segments_read += bytes_to_read;
            fragementation = (pageAlloc_counter * page_size) - segments_read;

            // keep record of which segment was touched
            segment_index[array_size++] = i;
            return;  
        }
    }

    // Invalid memory access
    fprintf(stderr, "Invalid memory access at address: %p\n", info->si_addr);
    exit(1);
}

void load_and_run_elf(char **exe) {
    fd = open(exe[1], O_RDONLY);

    if (fd < 0) {
        perror("Opening Error");
        exit(1);
    }

    // allocate memory 
    ehdr = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));
    if (!ehdr) {
        perror("ehdr allocation error");          // Error handled
        loader_cleanup();
        exit(1);
    }

    if (read(fd, ehdr, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) {
        perror("Reading ELF header error");       // Error handled
        loader_cleanup();
        exit(1);
    }

    // allocate memory for program headers and read them
    phdr = (Elf32_Phdr *)malloc(ehdr->e_phentsize * ehdr->e_phnum);
    if (!phdr) {
        perror("phdr allocation error");          // Error handled
        loader_cleanup();
        exit(1);
    }

    lseek(fd, ehdr->e_phoff, SEEK_SET);
    if (read(fd, phdr, ehdr->e_phentsize * ehdr->e_phnum) != (ehdr->e_phentsize * ehdr->e_phnum)) {
        perror("Reading program header error");   // Error handled
        loader_cleanup();
        exit(1);
    }

    // Setup SIGSEGV handler for page faults
    struct sigaction page_fault;
    memset(&page_fault, 0, sizeof(page_fault));
    page_fault.sa_sigaction = sigsegv_handler;
    page_fault.sa_flags = SA_SIGINFO;

    if (sigaction(SIGSEGV, &page_fault, NULL) == -1) {
        perror("Failed to set up SIGSEGV handler");  // Error handled
        loader_cleanup();
        exit(1);
    }

    // Starting the execution from the entry point (_start)
    int (*_start)() = (int (*)())ehdr->e_entry;
    int result = _start();

    float frag = fragementation/1024.0;   // conversion of fragmentations from Bytes to KB

    //the final output of the program containing:
    printf("_start return value: %d\n", result);
    printf("Number of Page Faults: %d\n", pageFault_counter);
    printf("Number of Pages Allocated: %d\n", pageAlloc_counter);
    printf("Internal Fragmentations: %f KB\n", frag);
}

int main(int argc, char **argv) {

    // if only loader is run without any helper files like fib.c & sum.c
    if (argc != 2) {
        printf("Usage: %s <ELF Executable>\n", argv[0]);
        exit(1);
    }
    segment_index = (int *)malloc(100 * sizeof(int));

    load_and_run_elf(argv);
    loader_cleanup();
    return 0;
}
