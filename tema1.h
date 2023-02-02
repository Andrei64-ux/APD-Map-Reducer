#ifndef _TEMA1_H
#define _TEMA1_H 

#include <iostream>
#include <pthread.h>
#include <vector>
#include <algorithm>
#include <string.h>
#include <math.h>

#define DIE(assertion, call_description)	\
	do {									\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",	\
					__FILE__, __LINE__);	\
			perror(call_description);		\
			exit(EXIT_FAILURE);				\
		}									\
	} while(0)

using namespace std;

typedef struct struct_for_threads{
    int id;
    int reduce;
    vector<string> *files;
    vector<vector<int>> *lists;
} struct_for_threads;

pthread_barrier_t wait_mappers;
pthread_mutex_t mutex;

void init(struct_for_threads *my_struct, int id, int reduce_number, vector<vector<int>> *list, vector<string> *files);
bool binarySearch(int left, int right, int number, int power);
void *f_map(void *arg);
void *f_reduce(void *arg);

#endif
