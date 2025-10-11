#ifndef DUMMY_MAIN_H
#define DUMMY_MAIN_H

#include <signal.h>
#include <unistd.h>
#include <stdio.h>

int dummy_main(int argc, char **argv);
int main(int argc, char **argv) { // paising process, schedular will resume it with SIGCONT when scheduled to run
    raise(SIGSTOP); 
    int ret = dummy_main(argc, argv); // execute the actual user main 
    return ret;
}
#define main dummy_main // rename main to dummy main
#endif /* DUMMY_MAIN_H */