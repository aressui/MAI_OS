#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <math.h>


typedef struct {
    int *array;
    int low;
    int cnt;
    int dir;
    int depth_limit; 
} ThreadArgs;

static void swap_elems(int *x, int *y) {
    int tmp = *x;
    *x = *y;
    *y = tmp;
}

static void bitonic_merge(int *data, int low, int cnt, int dir) {
    if (cnt > 1) {
        int k = cnt / 2;
        for (int i = low; i < low + k; i++) {
            if (dir == (data[i] > data[i + k])) {
                swap_elems(&data[i], &data[i + k]);
            }
        }
        bitonic_merge(data, low, k, dir);
        bitonic_merge(data, low + k, k, dir);
    }
}

static void bitonic_sort_sequential(int *data, int low, int cnt, int dir) {
    if (cnt > 1) {
        int k = cnt / 2;
        bitonic_sort_sequential(data, low, k, 1);
        bitonic_sort_sequential(data, low + k, k, 0);
        bitonic_merge(data, low, cnt, dir);
    }
}

static void *work(void *_args) {
    ThreadArgs *args = (ThreadArgs*)_args;
    int low = args->low;
    int cnt = args->cnt;
    int dir = args->dir;
    int depth = args->depth_limit;

    if (cnt <= 1) return NULL;

    int k = cnt / 2;

    if (depth > 0) {
        pthread_t thread1, thread2;
        ThreadArgs args1, args2;

        args1.array = args->array;
        args1.low = low;
        args1.cnt = k;
        args1.dir = 1; 
        args1.depth_limit = depth - 1;

        args2.array = args->array;
        args2.low = low + k;
        args2.cnt = k;
        args2.dir = 0;
        args2.depth_limit = depth - 1;

        pthread_create(&thread1, NULL, work, &args1);
        pthread_create(&thread2, NULL, work, &args2);

        pthread_join(thread1, NULL);
        pthread_join(thread2, NULL);

    } else {
        bitonic_sort_sequential(args->array, low, k, 1);
        bitonic_sort_sequential(args->array, low + k, k, 0);
    }

    bitonic_merge(args->array, low, cnt, dir);

    return NULL;
}

static double get_time_diff(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

int is_power_of_two(int n) {
    return (n > 0) && ((n & (n - 1)) == 0);
}

int main(int argc, char** argv) {
    if (argc != 3) {
        const char msg[] = "./program array_size max_threads\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    int n = atoi(argv[1]);
    int max_threads = atoi(argv[2]);

    if (n <= 0 || max_threads <= 0) {
        const char msg[] = "error: invalid input (must be > 0)\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    if (!is_power_of_two(n)) {
        const char msg[] = "error: array size must be a power of 2 \n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    int *array = malloc(n * sizeof(int));
    int *seq_array = malloc(n * sizeof(int));
    
    if (!array || !seq_array) {
        const char msg[] = "error: memory allocation failed\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));
    for (int i = 0; i < n; ++i) {
        array[i] = rand() % 10000;
        seq_array[i] = array[i];
    }

    char buf[256];

    struct timespec start_paral, end_paral;
    if (clock_gettime(CLOCK_MONOTONIC, &start_paral) != 0) {
        const char msg[] = "error: cant get start time\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    int depth_limit = (int)(log2(max_threads));

    pthread_t root_thread;
    ThreadArgs args;
    args.array = array;
    args.low = 0;
    args.cnt = n;
    args.dir = 1;
    args.depth_limit = depth_limit;

    pthread_create(&root_thread, NULL, work, &args);
    pthread_join(root_thread, NULL);

    if (clock_gettime(CLOCK_MONOTONIC, &end_paral) != 0) {
        const char msg[] = "error: cant get end time\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    double total_paral_time = get_time_diff(start_paral, end_paral);

    if (snprintf(buf, sizeof(buf), "\nparallel\nsort completed in %.6f sec\nmax threads param: %d (depth: %d), size: %d\n", 
                 total_paral_time, max_threads, depth_limit, n) >= 0) {
        write(STDOUT_FILENO, buf, strlen(buf));
    }

    free(array);
    return 0;
}
