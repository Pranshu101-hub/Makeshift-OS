#include <stdio.h>
#include <stdlib.h>
#include <elf.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <signal.h>
#include <ucontext.h>
#include <stdbool.h>

// #ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS 0x20

void load_and_run_elf(char ** exe);
void loader_cleanup();