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

// uses fgets to prevent buffer overflows and removes the trailing newline character.

char* read_cmdline() {
    char* line = malloc(sizeof(char) * MAX_CMD_LEN);
    if (!line) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    if (!fgets(line, MAX_CMD_LEN, stdin)) {
        printf("\n"); 
        exit(0);
    }
    line[strcspn(line, "\n")] = 0; //remove trailing \n
    return line;
}

//uses strtok to tokenize the input string by spaces. The last element of the args array is set to NULL as required by execvp.

void parse_arguments(char* line, char** args) {
    int i = 0;
    char* token = strtok(line, " ");
    while (token != NULL && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;
}

// this function forks a new process. The child process executes the command using execvp. The parent process waits for the child to complete, records the execution time, 
// and adds the command to the history.

void exec_cmd(char** args, char* og_cmd) {
    struct timeval start, end;
    pid_t pid;
    gettimeofday(&start, NULL);
    pid = fork();
    if (pid == -1) {
        perror("fork failed");
        return;
    } else if (pid == 0) { // child process
        if (execvp(args[0], args) == -1) {
            perror("execvp failed");
            exit(EXIT_FAILURE);
        }
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        gettimeofday(&end, NULL);
        add_to_history(og_cmd, pid, start, end);
    }
}


// this function handles both single commands with pipes and multiple pipes. It creates a chain of child processes and connects their standard I/O using pipes.

void exec_pipecmd(char* command) {
    char* original_command = strdup(command);
    char* commands[MAX_ARGS];
    int num_commands = 0;
    
    // 1. Split the command string by '|' to get individual commands
    char* token = strtok(command, "|");
    while(token != NULL) {
        commands[num_commands++] = token;
        token = strtok(NULL, "|");
    }

    struct timeval start, end;
    gettimeofday(&start, NULL);

    int input_fd = STDIN_FILENO; // The input for the first command is stdin
    pid_t pids[num_commands];

    // 2. Loop through each command and set up the pipeline
    for (int i = 0; i < num_commands; i++) {
        int pipefd[2];

        // Create a pipe for all but the last command
        if (i < num_commands - 1) {
            if (pipe(pipefd) == -1) {
                perror("pipe failed");
                free(original_command);
                return;
            }
        }

        pids[i] = fork();
        if (pids[i] < 0) {
            perror("fork failed");
            free(original_command);
            return;
        }

        if (pids[i] == 0) {
            // child
            if (input_fd != STDIN_FILENO) { // redirect input if it's not the first command
                dup2(input_fd, STDIN_FILENO);
                close(input_fd);
            }

            // redirect output if it's not the last command
            if (i < num_commands - 1) {
                dup2(pipefd[1], STDOUT_FILENO);
                // child doesn't need the pipe endpoints after dup2
                close(pipefd[0]);
                close(pipefd[1]);
            }
            
            char* args[MAX_ARGS]; // parse and execute the current command
            parse_arguments(commands[i], args);
            if (execvp(args[0], args) == -1) {
                perror("execvp failed");
                exit(EXIT_FAILURE);
            }
        } 
        else {
            //parent
            // Close the previous input FD if it's not stdin
            if (input_fd != STDIN_FILENO) {
                close(input_fd);
            }
            
            // for the next iteration the input will be the read-end of the current pipe
            if (i < num_commands - 1) {
                close(pipefd[1]); // parent doesn't need the write-end
                input_fd = pipefd[0];
            }
        }
    }
    // 3. Wait for all child processes to finish
    for (int i = 0; i < num_commands; i++) {
        waitpid(pids[i], NULL, 0);
    }
    gettimeofday(&end, NULL);
    
    // Add to history, using the PID of the last command in the pipeline
    add_to_history(original_command, pids[num_commands - 1], start, end);
    free(original_command);
}

// continuously displays a prompt, reads user input, and dispatches the command for execution, handling builtin commands like 'history' and 'exit'.

void shell_loop() {
    char* line;
    char* args[MAX_ARGS];
    char* line_copy_for_history;

    signal(SIGINT, handle_sigint);

    while (1) {
        printf("ospansu:~$ ");
        line = read_cmdline();

        if (strlen(line) == 0) {
            free(line);
            continue;
        }
        line_copy_for_history = strdup(line);// handle built-in commands
        if (strcmp(line, "exit") == 0) { 
            free(line); 
            free(line_copy_for_history);
            break;
        }

        if (strcmp(line, "history") == 0) {
            // for the built-in 'history', we don't fork, so we manually create a history entry
            struct timeval start, end;
            gettimeofday(&start, NULL);
            display_history();
            gettimeofday(&end, NULL);
            add_to_history(line_copy_for_history, getpid(), start, end);
            free(line);
            free(line_copy_for_history);
            continue;
        }
        if (strchr(line, '|')) {
            exec_pipecmd(line);
        } else {
            parse_arguments(line, args);
            exec_cmd(args, line_copy_for_history);
        }
        free(line);
        free(line_copy_for_history);
    }
}

//main entry point of the program.
int main() {
    shell_loop();
    for (int i = 0; i < histc; i++) { //clean up allocated memory before exiting normally
        free(history_log[i].cmd_str);
    }
    return EXIT_SUCCESS;
}
