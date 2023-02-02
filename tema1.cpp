#include "tema1.h"

void init(struct_for_threads *my_struct, int id, int reduce_number, vector<vector<int>> *list, vector<string> *files) {
    my_struct->id = id;
    my_struct->reduce = reduce_number;
    my_struct->lists = list;
    my_struct->files = files;
}

bool binarySearch(int left, int right, int number, int power)
{
    while (right - left >= 0) {
        int mid = left + (right - left) / 2;
        if (pow(mid, power) == number)
            return true;
        if (pow(mid, power) > number) {
            right = mid - 1;
        } else {
            left = mid + 1;
        }
    }
    return false;
}

void *f_map(void *arg)
{
	struct_for_threads thread_struct = *(struct_for_threads *)arg;

    while (!thread_struct.files->empty()) {
        string file;
        
        pthread_mutex_lock(&mutex);
        
        if(thread_struct.files->empty()) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        file = thread_struct.files->front();
        thread_struct.files->erase(thread_struct.files->begin());

        pthread_mutex_unlock(&mutex);

        vector<int> numbers;
        FILE *input = fopen(file.data(), "r");

        char line[100];
        int size_numbers = atoi(fgets(line, 100, input));
        
        for (int i = 0; i < size_numbers ; i++) {
            string number = fgets(line, 100, input);
            number.erase(remove(number.begin(), number.end(), '\n'), number.end());
                if (atoi(number.data()) > 0) {
                    numbers.push_back(atoi(number.data()));
                }
        }
        fclose(input);

        vector<vector<int>> compute_lists;
        compute_lists.resize(thread_struct.reduce);

        bool is_power;
        for(int i = 0; i < (int)numbers.size(); i++) {
            for(int j = 0; j < thread_struct.reduce; j++) {
                is_power = binarySearch(1, sqrt(numbers[i]), numbers[i], j+2);
                if (is_power) {
                    compute_lists[j].push_back(numbers[i]);
                }
            }
        }

        pthread_mutex_lock(&mutex);
        for (int i = 0; i < (int)compute_lists.size(); i++) {
            thread_struct.lists->push_back(compute_lists[i]);
        }
        pthread_mutex_unlock(&mutex);
    }

    pthread_barrier_wait(&wait_mappers);
	pthread_exit(NULL);
}

void *f_reduce(void *arg)
{   
	struct_for_threads thread_struct = *(struct_for_threads *)arg;
    pthread_barrier_wait(&wait_mappers);

    char name[100];
    sprintf(name, "out%d.txt", thread_struct.id + 2);
    FILE *fp = fopen(name, "w");
    DIE(fp == NULL , "Error at file open!\n");

    vector<int> final_list;
    for (int i = thread_struct.id; i < (int)thread_struct.lists->size(); i+=thread_struct.reduce) {
        for (int j = 0; j < (int)thread_struct.lists->at(i).size(); j++) {
            final_list.push_back(thread_struct.lists->at(i)[j]);
        }
    }

    sort(final_list.begin(), final_list.end());
    int uniqueCount = unique(final_list.begin(), final_list.end()) - final_list.begin();
    fprintf(fp, "%d", uniqueCount);
    fclose(fp);

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    DIE(argc != 4, "Usage ./tema1 mapper reducer file");
    int map_number = atoi(argv[1]);
    int reduce_number = atoi(argv[2]);

    FILE *file;
    file = fopen(argv[3], "r");
    DIE(file == NULL, "Error at open file!\n");

    char line[100];
    vector<string> files_list;

    pthread_t threads_struct[map_number + reduce_number];
    void *status;

    int files_number = atoi(fgets(line, 100, file));

    string l;
	for (int i = 0; i < files_number; i++) {
        l = fgets(line, 100, file);
        l.erase(remove(l.begin(), l.end(), '\n'), l.end());
        files_list.push_back(l);
    }

    vector<string> mapper_files;
    for (int i = 0; i < files_number; i++)
        mapper_files.push_back(files_list[i]);

    DIE(pthread_barrier_init(&wait_mappers, NULL, map_number + reduce_number) != 0 , "Error can't initalize barrier!");
	DIE(pthread_mutex_init(&mutex, NULL) != 0, "Error can't initalize mutex!");

    vector<vector<int>> *lists_of_list = new vector<vector<int>>();

    struct_for_threads threads_arguments[map_number + reduce_number];

	for (int i = 0; i < map_number + reduce_number; i++) {
        if (i < map_number) {
            init(&threads_arguments[i], i, reduce_number, lists_of_list, &mapper_files);
		    int r = pthread_create(&threads_struct[i], NULL, f_map, &threads_arguments[i]);
            DIE(r , "Eroare la crearea thread-urilor");
        } else {
            init(&threads_arguments[i], i - map_number, reduce_number, lists_of_list, NULL);
		    int r = pthread_create(&threads_struct[i], NULL, f_reduce, &threads_arguments[i]);
            DIE(r , "Eroare la crearea thread-urilor");
        }
	}

	for (int i = 0; i < map_number + reduce_number; i++) {
        if (i < map_number) {
		    int r = pthread_join(threads_struct[i], &status);
            DIE(r , "Eroare la asteptarea thread-urilor");
        } else {
		    int r = pthread_join(threads_struct[i], &status);
            DIE(r , "Eroare la asteptarea thread-urilor");
        }
	}

    pthread_mutex_destroy(&mutex);
    pthread_barrier_destroy(&wait_mappers);
    delete lists_of_list;
    fclose(file);

    return 0;
}