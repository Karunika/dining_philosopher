#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define N 5
#define LEFT (i + N - 1) % N
#define RIGHT (i + 1) % N

#define THINKING 0
#define HUNGRY 1
#define EATING 2

pthread_mutex_t m;
pthread_mutex_t s[N];
int state[N];

void grab_forks(int i);
void put_away_forks(int i);
void test(int i);

void* philosopher(void* arg) {
    int id = *((int*)arg);
    free(arg);

    for (int j = 0; j < 5; j++) {
        printf("Philosopher %d is thinking\n", id);

        grab_forks(id);

        printf("Philosopher %d is eating\n", id);

        put_away_forks(id);
    }

    return NULL;
}

void grab_forks(int i) {
    pthread_mutex_lock(&m);
    state[i] = HUNGRY;
    test(i);
    pthread_mutex_unlock(&m);
    pthread_mutex_lock(&s[i]);
}

void put_away_forks(int i) {
    pthread_mutex_lock(&m);
    state[i] = THINKING;
    test(LEFT);
    test(RIGHT);
    pthread_mutex_unlock(&m);
}

void test(int i) {
    if (state[i] == HUNGRY && state[LEFT] != EATING && state[RIGHT] != EATING) {
        state[i] = EATING;
        pthread_mutex_unlock(&s[i]);
    }
}

int main() {
    pthread_t philosophers[N];
    pthread_mutex_init(&m, NULL);

    for (int i = 0; i < N; i++) {
        pthread_mutex_init(&s[i], NULL);
        pthread_mutex_lock(&s[i]);
        state[i] = THINKING;
    }

    for (int i = 0; i < N; i++) {
        int* arg = malloc(sizeof(*arg));
        *arg = i;
        pthread_create(&philosophers[i], NULL, philosopher, arg);
    }

    for (int i = 0; i < N; i++) {
        pthread_join(philosophers[i], NULL);
        pthread_mutex_destroy(&s[i]);
    }

    pthread_mutex_destroy(&m);

    return 0;
}
