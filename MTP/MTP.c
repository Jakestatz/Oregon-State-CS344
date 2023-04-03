#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

// Size of the buffers
#define SIZE 50
#define SIZE2 1000

// Buffer 1, shared resource between input thread and line separator thread
char buffer_1[SIZE][SIZE2];
// Number of items in the buffer
int count_1 = 0;
// Index where the input thread will put the next item
int prod_idx_1 = 0;
// Index where the square-root thread will pick up the next item
int con_idx_1 = 0;
// Initialize the mutex for buffer 1
pthread_mutex_t mutex_1 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 1
pthread_cond_t full_1 = PTHREAD_COND_INITIALIZER;


// Buffer 2, shared resource between line separator thread and plus sign thread
char buffer_2[SIZE][SIZE2];
// Number of items in the buffer
int count_2 = 0;
// Index where the line-separator thread will put the next item
int prod_idx_2 = 0;
// Index where the output thread will pick up the next item
int con_idx_2 = 0;
// Initialize the mutex for buffer 2
pthread_mutex_t mutex_2 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 2
pthread_cond_t full_2 = PTHREAD_COND_INITIALIZER;


// Buffer 3, shared resource between plus sign thread and output thread
char buffer_3[SIZE][SIZE2];
// Number of items in the buffer
int count_3 = 0;
// Index where the plus-sign thread will put the next item
int prod_idx_3 = 0;
// Index where the output thread will pick up the next item
int con_idx_3 = 0;
// Initialize the mutex for buffer 3
pthread_mutex_t mutex_3 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 3
pthread_cond_t full_3 = PTHREAD_COND_INITIALIZER;


/*
Get input from the user.
This function doesn't perform any error checking.
*/
char* get_user_input() {
    char* input = NULL;
    size_t len = 0;
    getline(&input, &len, stdin);
    return input;
}

/*
 Put an item in buff_1
*/
void put_buff_1(char* item) {
    // Lock the mutex before putting the item in the buffer
    pthread_mutex_lock(&mutex_1);
    // Put the item in the buffer
    strcpy(buffer_1[prod_idx_1], item);
    // Increment the index where the next item will be put.
    prod_idx_1 = prod_idx_1 + 1;
    count_1++;
    // Signal to the consumer that the buffer is no longer empty
    pthread_cond_signal(&full_1);
    // Unlock the mutex
    pthread_mutex_unlock(&mutex_1);
}

/*
 Function that the input thread will run.
 Get input from the user.
 Put the item in the buffer shared with the square_root thread.
*/
void* get_input(void* args) {
    for (int i = 0; i < SIZE; i++) {
        // Get the user input
        char* input = get_user_input();
        if (!strcmp(input, "STOP\n")) {
            put_buff_1(input);
            return NULL;
        }
        put_buff_1(input);
        free(input);
    }
    return NULL;
}

/*
Get the next item from buffer 1
*/
char* get_buff_1() {
    // Lock the mutex before checking if the buffer has data
    pthread_mutex_lock(&mutex_1);
    while (count_1 == 0)
        // Buffer is empty. Wait for the producer to signal that the buffer has data
        pthread_cond_wait(&full_1, &mutex_1);
    char* item = buffer_1[con_idx_1];
    // Increment the index from which the item will be picked up
    con_idx_1 = con_idx_1 + 1;
    count_1--;
    // Unlock the mutex
    pthread_mutex_unlock(&mutex_1);
    // Return the item
    return item;
}

/*
 Put an item in buff_2
*/
void put_buff_2(char* item) {
    // Lock the mutex before putting the item in the buffer
    pthread_mutex_lock(&mutex_2);
    // Put the item in the buffer
    strcpy(buffer_2[prod_idx_2], item);
    // Increment the index where the next item will be put.
    prod_idx_2 = prod_idx_2 + 1;
    count_2++;
    // Signal to the consumer that the buffer is no longer empty
    pthread_cond_signal(&full_2);
    // Unlock the mutex
    pthread_mutex_unlock(&mutex_2);
}

/*
 Function that the line separator thread will run.
 Consume an item from the buffer shared with the input thread.
 Replace newlines with spaces.
 Produce an item in the buffer shared with the plus sign thread.
*/
void* line_separator(void* args) {
    char* item;
    for (int i = 0; i < SIZE; i++) {
        item = get_buff_1();
        if (!strcmp(item, "STOP\n")) {
            put_buff_2(item);
            return NULL;
        }
        for (int j = 0; j < strlen(item); j++) {
            if (item[j] == '\n') {
                item[j] = ' ';
            }
        }
        put_buff_2(item);
    }
    return NULL;
}

/*
Get the next item from buffer 2
*/
char* get_buff_2() {
    // Lock the mutex before checking if the buffer has data
    pthread_mutex_lock(&mutex_2);
    while (count_2 == 0)
        // Buffer is empty. Wait for the producer to signal that the buffer has data
        pthread_cond_wait(&full_2, &mutex_2);
    char* item = buffer_2[con_idx_2];
    // Increment the index from which the item will be picked up
    con_idx_2 = con_idx_2 + 1;
    count_2--;
    // Unlock the mutex
    pthread_mutex_unlock(&mutex_2);
    // Return the item
    return item;
}

/*
 Put an item in buff_3
*/
void put_buff_3(char* item) {
    // Lock the mutex before putting the item in the buffer
    pthread_mutex_lock(&mutex_3);
    // Put the item in the buffer
    strcpy(buffer_3[prod_idx_3], item);
    // Increment the index where the next item will be put.
    prod_idx_3 = prod_idx_3 + 1;
    count_3++;
    // Signal to the consumer that the buffer is no longer empty
    pthread_cond_signal(&full_3);
    // Unlock the mutex
    pthread_mutex_unlock(&mutex_3);
}

/*
 Function that the plus sign thread will run.
 Consume an item from the buffer shared with the line separator thread.
 Replace ++ with ^.
 Produce an item in the buffer shared with the output thread.
*/
void* plus_sign(void* args) {
    char* item;
    while(1) {
        item = get_buff_2();
        if (!strcmp(item, "STOP\n")) {
            put_buff_3(item);
            return NULL;
        }
        for (int j = 0; j < strlen(item); j++) {
            if (item[j] == '+' && item[j + 1] == '+') {
                item[j] = '^';
                for (int k = j + 1; k < SIZE2; k++) {
                    item[k] = item[k + 1];
                }
            }
        }

        put_buff_3(item);
    }
    return NULL;
}

char* get_buff_3() {
    // Lock the mutex before checking if the buffer has data
    pthread_mutex_lock(&mutex_3);
    while (count_3 == 0)
        // Buffer is empty. Wait for the producer to signal that the buffer has data
        pthread_cond_wait(&full_3, &mutex_3);
    char* item = buffer_3[con_idx_3];
    // Increment the index from which the item will be picked up
    con_idx_3 = con_idx_3 + 1;
    count_3--;
    // Unlock the mutex
    pthread_mutex_unlock(&mutex_3);
    // Return the item
    return item;
}

/*
 Function that the output thread will run.
 Consume an item from the buffer shared with the plus sign thread.
 Print the item.
*/
void* write_output(void* args) {
    char buff[80];
    char* item;
    int charc = 0;
    while (1) {
        item = get_buff_3();
        for (int i = 0; i < strlen(item); i++) {
            buff[charc] = item[i];
            charc++;
            if (charc == 80) {
                charc = 0;
                // .80 used becuase garbage was printed after line
                printf("%.80s\n", buff);
                fflush(stdout);
            }
        }
        if (!strcmp(item, "STOP\n")) {
            return NULL;
        }
    }
    return NULL;
}

int main(int argc, const char* argv[]) {
    pthread_t input_t, line_separator_t, plus_sign_t, output_t;
    // Create the threads
    pthread_create(&input_t, NULL, get_input, NULL);
    pthread_create(&line_separator_t, NULL, line_separator, NULL);
    pthread_create(&plus_sign_t, NULL, plus_sign, NULL);
    pthread_create(&output_t, NULL, write_output, NULL);
    // Wait for the threads to terminate
    pthread_join(input_t, NULL);
    pthread_join(line_separator_t, NULL);
    pthread_join(plus_sign_t, NULL);
    pthread_join(output_t, NULL);
    return EXIT_SUCCESS;
}