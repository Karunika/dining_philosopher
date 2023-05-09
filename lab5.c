#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>

#define NUM_PHILOSOPHERS 5

int sem_id;

void grab_forks(int left_fork_id);
void put_away_forks(int left_fork_id);
void philosopher(int id);

int main() {
    int i;
    pid_t pid;

    sem_id = semget(IPC_PRIVATE, NUM_PHILOSOPHERS, 0666 | IPC_CREAT);
    if (sem_id == -1) {
        perror("semget");
        exit(1);
    }

    for (i = 0; i < NUM_PHILOSOPHERS; i++) {
        if (semctl(sem_id, i, SETVAL, 1) == -1) {
            perror("semctl");
            exit(1);
        }
    }

    for (i = 0; i < NUM_PHILOSOPHERS; i++) {
        pid = fork();

        if (pid == -1) {
            perror("fork");
            exit(1);
        } else if (pid == 0) {
            philosopher(i);
            exit(0);
        }
    }

    while (wait(NULL) != -1);

    if (semctl(sem_id, 0, IPC_RMID) == -1) {
        perror("semctl");
        exit(1);
    }

    return 0;
}

void grab_forks(int left_fork_id) {
    struct sembuf sops[2];

    sops[0].sem_num = left_fork_id;
    sops[0].sem_op = -1;
    sops[0].sem_flg = SEM_UNDO;

    sops[1].sem_num = (left_fork_id + 1) % NUM_PHILOSOPHERS;
    sops[1].sem_op = -1;
    sops[1].sem_flg = SEM_UNDO;

    if (semop(sem_id, sops, 2) == -1) {
        perror("semop");
        exit(1);
    }
}

void put_away_forks(int left_fork_id) {
    struct sembuf sops[2];

    sops[0].sem_num = left_fork_id;
    sops[0].sem_op = 1;
    sops[0].sem_flg = SEM_UNDO;

    sops[1].sem_num = (left_fork_id + 1) % NUM_PHILOSOPHERS;
    sops[1].sem_op = 1;
    sops[1].sem_flg = SEM_UNDO;

    if (semop(sem_id, sops, 2) == -1) {
        perror("semop");
        exit(1);
    }
}

void philosopher(int id) {
    for (int i = 0; i < 5; i++) {
        printf("Philosopher %d is thinking\n", id);

        grab_forks(id);

        printf("Philosopher %d is eating\n", id);

        put_away_forks(id);
    }
}
