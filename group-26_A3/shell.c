#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define MAX_CMD_LEN 1024
#define MAX_JOBS 100

typedef struct {
    int ncpu;
    int tslice;
    pid_t scheduler_pid;
    int jobc;
    pid_t job_pids[MAX_JOBS];
    char job_names[MAX_JOBS][256];
    int job_completion_time[MAX_JOBS];
    int job_wait_time[MAX_JOBS];
    int job_finished[MAX_JOBS];
    int shutdown;
} scheduler_data;

// global variables for scheduler interaction
int shmid = -1;
scheduler_data *sched_data = NULL;
pid_t scheduler_pid = -1;

void cleanup();

void handle_sigint(int sig) {
    (void)sig; // Suppress unused parameter warning
    printf("\nCaught SIGINT. Shutting down gracefully...\n");
    cleanup();
    exit(0);
}

char* read_cmdline() {
    char* line = malloc(sizeof(char) * MAX_CMD_LEN);
    if (!line) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    // if (!fgets(line, MAX_CMD_LEN, stdin)) {
    //     printf("\n"); // Handle Ctrl+D (EOF)
    //     cleanup();
    //     exit(0);
    // }
    line[strcspn(line, "\n")] = 0; // remove trailing newline
    return line;
}


void submit_job(char* executable) {
    if (sched_data->jobc >= MAX_JOBS) {
        fprintf(stderr, "Error: Maximum job limit reached.\n");
        return;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed for job submission");
        return;
    }
    
    if (pid == 0) {
        // fork child process 
        char* args[] = {executable, NULL};
        execv(args[0], args);
        perror("execv failed"); // if execv fails
        exit(EXIT_FAILURE);
    } else {
        // record new job
        int job_idx = sched_data->jobc;
        sched_data->job_pids[job_idx] = pid;
        strncpy(sched_data->job_names[job_idx], executable, 255);
        sched_data->job_names[job_idx][255] = '\0';
        sched_data->jobc++;
        
        printf("Job '%s' submitted with PID %d.\n", executable, pid);
    }
}

void shell_loop() {
    char* line;
    signal(SIGINT, handle_sigint);

    while (1) {
        printf("ospansu!$ ");
        line = read_cmdline();

        if (strlen(line) == 0) {
            free(line);
            continue;
        }

        if (strcmp(line, "exit") == 0) {
            free(line);
            break; 
        }

        if (strncmp(line, "submit ", 7) == 0) {
            char* executable = line + 7;
            while (*executable == ' ') executable++; 
            if (strlen(executable) > 0) {
                 submit_job(executable);
            } else {
                 fprintf(stderr, "Usage: submit <path_to_executable>\n");
            }
        } else {
            fprintf(stderr, "Unknown command. Use 'submit <executable>' or 'exit'.\n");

        }
        
        free(line);
    }
}

void init_scheduler(int ncpu, int tslice) {
    // create a key for the shm
    key_t key = ftok("shell.c", 'S');
    if (key == -1) {
        perror("ftok");
        exit(1);
    }

    // create the shm
    shmid = shmget(key, sizeof(scheduler_data), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(1);
    }

    // attach to the shm
    sched_data = (scheduler_data*)shmat(shmid, NULL, 0);
    if (sched_data == (void*)-1) {
        perror("shmat");
        exit(1);
    }

    // initialize the shared data.
    sched_data->ncpu = ncpu;
    sched_data->tslice = tslice;
    sched_data->jobc = 0;
    sched_data->shutdown = 0;

    // fork and start the scheduler process.
    scheduler_pid = fork();
    if (scheduler_pid == -1) {
        perror("fork for scheduler");
        exit(1);
    } else if (scheduler_pid == 0) {
        // Child process becomes the scheduler.
        execl("./simplescheduler", "simplescheduler", NULL);
        perror("execl for scheduler failed"); 
        exit(1);
    }
    sched_data->scheduler_pid = scheduler_pid;
    usleep(100000); // give scheduler time to initialize.
}

void cleanup() {
    if (sched_data != NULL) {
        printf("Initiating shutdown. Waiting for all jobs to complete...\n");
        sched_data->shutdown = 1;
        if (scheduler_pid > 0) {
			while (waitpid(scheduler_pid, NULL, WNOHANG) == 0) {
				waitpid(-1, NULL, WNOHANG);
				usleep(100000); 
			}
		}

        // print the final job history
        printf("\n--- Job History ---\n");
        for (int i = 0; i < sched_data->jobc; i++) {
            int completion = sched_data->job_completion_time[i] > 0 ? sched_data->job_completion_time[i] : 1;
            printf("Job: %s (PID: %d), Completion Time: %d x TSLICE, Wait Time: %d x TSLICE\n",
                   sched_data->job_names[i], sched_data->job_pids[i],
                   completion, sched_data->job_wait_time[i]);
        }
        printf("----------------------\n");

        shmdt(sched_data);
        shmctl(shmid, IPC_RMID, NULL);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <NCPU> <TSLICE_ms>\n", argv[0]);
        return 1;
    }

    int ncpu = atoi(argv[1]);
    int tslice = atoi(argv[2]);

    if (ncpu <= 0 || tslice <= 0) {
        fprintf(stderr, "NCPU and TSLICE must be positive integers.\n");
        return 1;
    }

    printf("Starting simpleShell with NCPU=%d, TSLICE=%dms\n", ncpu, tslice);
    init_scheduler(ncpu, tslice);
    shell_loop();
    cleanup();

    return EXIT_SUCCESS;
}
