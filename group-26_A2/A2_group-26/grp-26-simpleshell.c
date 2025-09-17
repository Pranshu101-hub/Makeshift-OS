#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <signal.h>
#include <time.h>

#define MAX_CMD_LEN 1024 //defined for buffer size
#define MAX_ARGS 1000
#define MAX_HISTORY 1000

//this struct encapsulates the command string, its process ID (PID), the time it started, and its total execution duration.

typedef struct {
    char* cmd_str;              // command string entered by the user
    pid_t pid;                  // process ID of the executed command
    double start_time;          // timestamp when the command started
    double execution_duration;  // how long the command took to run, in seconds
} cmdent;

// global array to store the history of commands as an array of structs, and history counts
cmdent history_log[MAX_HISTORY];
int histc = 0;

//prints the local time from a given tstamp.
void print_formatted_time(double tstamp) {
    time_t raw = (time_t)tstamp;
    struct tm *time_info = localtime(&raw);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", time_info);
    printf("%s", buffer);
}

// manages the history log as a circular buffer. If the log is full, it discards the oldest entry to make room for the new one.
 
void add_to_history(char* command, pid_t pid, struct timeval start, struct timeval end) {
    if (histc < MAX_HISTORY) {
        history_log[histc].cmd_str = strdup(command);
        history_log[histc].pid = pid;
        history_log[histc].start_time = start.tv_sec + start.tv_usec / 1e6;
        history_log[histc].execution_duration = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
        histc++;
    } 
    else {
        perror("MAX HISTORY MEMORY LIMIT EXCEEDED."); //history full
    }
}

//iterates through the global history_log and prints the serial number,PID, start time, execution duration, and the command itself for each entry.

void display_history() {
    printf("\n--- Command History ---\n");
    for (int i = 0; i < histc; i++) {
        printf("%d: PID=%d, ", i + 1, history_log[i].pid);
        printf("Start: ");
        print_formatted_time(history_log[i].start_time);
        printf(", Duration: %.4f s, ", history_log[i].execution_duration);
        printf("Cmd: \"%s\"\n", history_log[i].cmd_str);
    }
    printf("-----------------\n");
}


//this function is called when the user presses Ctrl+C. It prints a message, displays the command history, and exits the shell gracefully.
 
void handle_sigint(int sig) {
    printf("\nCaught SIGINT. Displaying history and exiting.\n");
    display_history();
    for (int i = 0; i < histc; i++) {     // free dynamically allocated memory in history
        free(history_log[i].cmd_str);
    }
    exit(0);
}
