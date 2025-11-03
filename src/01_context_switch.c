#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <x86intrin.h>
#include <emmintrin.h>

#include "00_function_call.h"
#include "stats.h"
#include "01_context_switch.h"

calibrated_stats contextswitch_syscall(int iterations) {
    int n_extra = iterations / 10;
    iterations += n_extra;
    int *times = malloc(sizeof(int) * iterations);
    int *times_calib = malloc(sizeof(int) * iterations);
    long long start, end;
    unsigned int tsc_aux;
    for (int i = 0; i < iterations; i++) {
        start = __rdtscp(&tsc_aux);
        _mm_lfence();
        getpid();
        end = __rdtscp(&tsc_aux);
        _mm_lfence();
        times[i] = end - start;
    }
    for (int i = 0; i < iterations; i++) {
        start = __rdtscp(&tsc_aux);
        _mm_lfence();
        function_i_v();
        end = __rdtscp(&tsc_aux);
        _mm_lfence();
        times_calib[i] = end - start;
    }
    // Skip the first 10th of the tests as "warmup"
    int n_valid_test = iterations - n_extra;
    calibrated_stats s = {int_stats(times_calib + n_extra, n_valid_test),
                          int_stats(times + n_extra, n_valid_test)};

    free(times);
    free(times_calib);
    times = NULL;
    times_calib = NULL;

    return s;
}

// The semaphore to ping/pong on.
static sem_t sem;
// To make the threads synchronize their starts.
static pthread_barrier_t barrier;

static int n_test;
static long long *starts;
static long long *ends;

calibrated_stats contextswitch_thread(int iterations) {
    // Skip the first 10th of the tests as "warmup"
    int n_extra = iterations / 10;
    n_test = iterations + n_extra;
    // Setup the semaphore for inter-thread (but not inter-process) sharing
    sem_init(&sem, 0, 0);
    pthread_barrier_init(&barrier, NULL, 2);
    starts = malloc(sizeof(long long) * n_test);
    ends = malloc(sizeof(long long) * n_test);
    // For holding differences. The differences will be much smaller than
    // INTMAX.
    int *times = malloc(sizeof(int) * n_test);
    // This one is for the calibration run. It doesn't need explicit start and
    // end arrays; it will only be running on one thread.
    int *times_calib = malloc(sizeof(int) * n_test);

    // Start the experiment!
    pthread_t pingThread, pongThread;
    pthread_create(&pingThread, NULL, &contextswitch_ping, NULL);
    pthread_create(&pongThread, NULL, &contextswitch_pong, NULL);

    // Wait for it to finish before starting the calibration run.
    pthread_join(pingThread, NULL);
    pthread_join(pongThread, NULL);

    for (int i = 0; i < n_test; i++) {
        times[i] = ends[i] - starts[i];
    }

    free(starts);
    free(ends);
    starts = NULL;
    ends = NULL;

    contextswitch_pingpong(times_calib);

    int n_valid_test = n_test - n_extra;
    calibrated_stats s = {int_stats(times_calib + n_extra, n_valid_test),
                          int_stats(times + n_extra, n_valid_test)};

    free(times);
    free(times_calib);
    times = NULL;
    times_calib = NULL;

    return s;
}

void *contextswitch_ping(void *ptr) {
    (void)ptr;
    unsigned int tsc_aux;
    unsigned long long tsc;

    for (long i = 0; i < n_test; i++) {
        // Synchronize up with the other thread to have a consistent start
        // point.
        pthread_barrier_wait(&barrier);
        // Let the other thread wait on the semaphore.
        usleep(1000);
        // Start the clock!
        tsc = __rdtscp(&tsc_aux);
        // For consistent instruction ordering.
        _mm_lfence();
        // Wakeup the other thread.
        sem_post(&sem);
        // Let the other thread get our message, please.
        usleep(1000);
        starts[i] = tsc;
    }
    return NULL;
}

void *contextswitch_pong(void *ptr) {
    (void)ptr;
    unsigned int tsc_aux;
    unsigned long long tsc;

    for (long i = 0; i < n_test; i++) {
        // Synchronize up with the other thread to have a consistent start
        // point.
        pthread_barrier_wait(&barrier);
        // Wait on the semaphore.
        sem_wait(&sem);
        // Stop the clock.
        tsc = __rdtscp(&tsc_aux);
        // For consistent instruction ordering.
        _mm_lfence();
        ends[i] = tsc;
    }
    return NULL;
}

void contextswitch_pingpong(int *store) {
    unsigned int tsc_aux;
    unsigned long long tsc_start;
    unsigned long long tsc_end;
    for (long i = 0; i < n_test; i++) {
        tsc_start = __rdtscp(&tsc_aux);
        _mm_lfence();
        // These should return instantly without incurring a context switch.
        sem_post(&sem);
        // We are incurring a function call overhead here. This is fine; the
        // real thing has a usleep() function call to match. And this way we get
        // to sample the consequences of performing sem_wait's cleanup.
        sem_wait(&sem);
        tsc_end = __rdtscp(&tsc_aux);
        _mm_lfence();
        store[i] = tsc_end - tsc_start;
    }
}
