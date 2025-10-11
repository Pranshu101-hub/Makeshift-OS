#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>

#define MAX_JOBS 100

typedef struct { // shm structure matches shell.c
    int ncpu;
    int tslice;
    pid_t scheduler_pid;
    int jobc;
    pid_t job_pids[MAX_JOBS];
    char job_names[MAX_JOBS][256];
    int complete_time[MAX_JOBS];
    int wait_time[MAX_JOBS];
    int job_finished[MAX_JOBS];
    int shutdown;
} scheduler_data;

typedef enum { // internal state for each job tracked by the scheduler
    READY,
    RUNNING,
    FINISHED
} job_status;

typedef struct { // state structure
    job_status status;
    int time_slices_used;
    int total_wait_time;
} job_state;

scheduler_data *sched_data = NULL;
job_state job_states[MAX_JOBS];
int ready_queue[MAX_JOBS];
int queue_head = 0;
int queue_tail = 0;
int jobs_in = 0;

void cleanup_and_exit(int sig) {
    if (sched_data != NULL) {
        shmdt(sched_data);
    }
    exit(0);
}

void init_shared_memory() {
    key_t key = ftok("shell.c", 'S');
    int shmid = shmget(key, sizeof(scheduler_data), 0666);
    if (shmid < 0) {
        perror("shmget failed in scheduler");
        exit(1);
    }
    
    sched_data = (scheduler_data*)shmat(shmid, NULL, 0);
    if (sched_data == (void*)-1) {
        perror("shmat failed in scheduler");
        exit(1);
    }
    
    for (int i = 0; i < MAX_JOBS; i++) {
        job_states[i].status = FINISHED; // initially no jobs
    }
}

void enqueue(int job_idx) { //add job idex behind the ready queue
    ready_queue[queue_tail] = job_idx;
    queue_tail = (queue_tail + 1) % MAX_JOBS;
}

// removes a job index from front of the ready queue
int dequeue() {
    if (queue_head == queue_tail){
        return -1;
    }               // empty
    int job_idx = ready_queue[queue_head];
    queue_head = (queue_head + 1) % MAX_JOBS;
    return job_idx;
}

int is_empty() {
    return queue_head == queue_tail;
}

void round_robin_schedule() {
    int list_runningjob[sched_data->ncpu];
    for (int i = 0; i < sched_data->ncpu; i++){
        list_runningjob[i] = -1;
    }
    int old_jobc = 0; //last known job count
    while (!sched_data->shutdown || jobs_in > 0) { //looping till shutdown or finish
        // check for new submitted jobs from the shell
        if (sched_data->jobc > old_jobc) {
            for (int i = old_jobc; i < sched_data->jobc; i++) {
                job_states[i].status = READY;
                job_states[i].time_slices_used = 0;
                job_states[i].total_wait_time = 0;
                enqueue(i);
                jobs_in++;
            }
            old_jobc = sched_data->jobc;
        }
                                                                
        for (int cpu = 0; cpu < sched_data->ncpu; cpu++) {// schedule jobs from the ready queue onto cpus
            if (list_runningjob[cpu] == -1 && !is_empty()) {
                int job_idx = dequeue();
                if (job_idx != -1) {
                    list_runningjob[cpu] = job_idx;
                    job_states[job_idx].status = RUNNING;
                    kill(sched_data->job_pids[job_idx], SIGCONT);
                }
            }
        }

        usleep(sched_data->tslice * 1000); // wait for one time slice

        for (int cpu = 0; cpu < sched_data->ncpu; cpu++) { // force exit running jobs and check for completion
            int job_idx = list_runningjob[cpu];
            if (job_idx != -1) {
                // use kill(pid, 0) to check if the process exists, returns -1 if not
                if (kill(sched_data->job_pids[job_idx], 0) == -1) {
                    // job has finished
                    job_states[job_idx].status = FINISHED;
                    sched_data->job_finished[job_idx] = 1;
                    job_states[job_idx].time_slices_used+=1; // count the last slice
                    sched_data->complete_time[job_idx] = job_states[job_idx].time_slices_used;
                    sched_data->wait_time[job_idx] = job_states[job_idx].total_wait_time;
                    jobs_in--;
                } else {
                    // force exit it with SIGSTOP
                    kill(sched_data->job_pids[job_idx], SIGSTOP);
                    job_states[job_idx].time_slices_used++;
                    job_states[job_idx].status = READY;
                    enqueue(job_idx); // add it back to the ready queue
                }
                list_runningjob[cpu] = -1; // free cpu
            }
        }    
        if (!is_empty()) {
            int current = queue_head;
            while (current != queue_tail) {
                job_states[ready_queue[current]].total_wait_time++;  // update wait time for all jobs in ready
                current = (current + 1) % MAX_JOBS;
            }
        }
    }
}

int main() {
    signal(SIGTERM, cleanup_and_exit);
    signal(SIGINT, cleanup_and_exit);
    init_shared_memory();
    round_robin_schedule();
    cleanup_and_exit(0);
    return 0;
}
