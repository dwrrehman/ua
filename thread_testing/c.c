#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdatomic.h>
#include <iso646.h>
#include <stdbool.h>

typedef long long nat;

static nat thread_count = 9;
static const nat job_count = 10000000;

struct thread_arguments {
	nat index;
};

static pthread_mutex_t input_mutex;
static pthread_mutex_t output_mutex;

static _Atomic nat output_count = 0; 
static _Atomic nat input_count = 0;

static nat* output = NULL; 
static nat* input = NULL;

static void* worker_thread(void* __attribute__((unused)) raw) {
	// struct thread_arguments args = * (struct thread_arguments*) raw;	

	while (true) {

		pthread_mutex_lock(&input_mutex);
		if (not input_count) { pthread_mutex_unlock(&input_mutex); break; }
		nat job = input[--input_count];
		pthread_mutex_unlock(&input_mutex);

		//printf("thread #[%llu]: processing job value = %llu\n", args.index, job);

		for (nat i = 0; i < job; i++) {
			job += rand() % 2;
		}

		pthread_mutex_lock(&output_mutex);
		output[output_count++] = job % 30;
		pthread_mutex_unlock(&output_mutex);
	}

	return NULL;
}

int main(int argc, const char** argv) {
	if (argc == 1) return puts("give the thread count as an argument!");

	thread_count = atoi(argv[1]);

	srand(42);

	clock_t begin = clock();

	pthread_mutex_init(&output_mutex, NULL);
	pthread_mutex_init(&input_mutex, NULL);

	input = calloc(job_count, sizeof(nat));
	output = calloc(job_count, sizeof(nat));
	
	for (nat i = 0; i < job_count; i++) 
		input[input_count++] = (nat) (rand() % 400);
	
	pthread_t* thread = calloc((size_t) thread_count, sizeof(pthread_t));

	for (nat i = 0; i < thread_count; i++) {
		struct thread_arguments* args = calloc(1, sizeof(struct thread_arguments));
		args->index = i;
		pthread_create(thread + i, NULL, worker_thread, args);
	}

	for (nat i = 0; i < thread_count; i++) 
		pthread_join(thread[i], NULL);

	printf("info: number of jobs completed = %llu\n", output_count);

	pthread_mutex_destroy(&output_mutex);
	pthread_mutex_destroy(&input_mutex);

	free(input);
	free(output);

	clock_t end = clock();
	double total_time = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("\t time for %llu threads:   %10.10lfs\n", thread_count, total_time);
}








// atomic_fetch_add_explicit(&acnt, 1, memory_order_relaxed);

