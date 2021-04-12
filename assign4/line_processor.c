#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

/*
  Assignment 4 CS344
  This code is largely based off the example code provided
  Author: Kris Haro
  Description: This program will create 4 threads to process input
  from stdin that communicate with each other using the 
  producer-consumer approach
*/

// Size of the buffers 49 lines of 10,000 characters ~ 50,000 chars
#define SIZE 50000

// Maximum size of the input line
#define MAX_LINE_SIZE 1000

// Output size
#define	outSize 80

// End marker to terminate threads, from example 6_4
#define END_MARKER -1

// Buffer 1, shared resource between input thread and square-root thread
int buffer_1[SIZE];
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


// Buffer 2, shared resource between square root thread and output thread
int buffer_2[SIZE];
// Number of items in the buffer
int count_2 = 0;
// Index where the square-root thread will put the next item
int prod_idx_2 = 0;
// Index where the output thread will pick up the next item
int con_idx_2 = 0;
// Initialize the mutex for buffer 2
pthread_mutex_t mutex_2 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 2
pthread_cond_t full_2 = PTHREAD_COND_INITIALIZER;

// Buffer 3, shared resource between square root thread and output thread
int buffer_3[SIZE];
// Number of items in the buffer
int count_3 = 0;
// Index where the square-root thread will put the next item
int prod_idx_3 = 0;
// Index where the output thread will pick up the next item
int con_idx_3 = 0;
// Initialize the mutex for buffer 2
pthread_mutex_t mutex_3 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 2
pthread_cond_t full_3 = PTHREAD_COND_INITIALIZER;


/*
 Put an item in buff_1
*/
void put_buff_1(int item) {
    // Lock the mutex before putting the item in the buffer
    pthread_mutex_lock(&mutex_1);
    // Put the item in the buffer
    buffer_1[prod_idx_1] = item;
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
 Put the item in the buffer shared with the line_separator thread.
*/
void* get_input(void* args)
{	int loophandle = 1;
	while(loophandle){ // loop forever to always be getting input
    char* theInput = malloc(MAX_LINE_SIZE * sizeof(char));
		fgets(theInput, MAX_LINE_SIZE, stdin);
		if(strcmp(theInput, "STOP\n") == 0){ // found our terminator, time to stop getting input
			put_buff_1(END_MARKER);
			loophandle = 0; // Break out the loop and terminate the other threads
		}
		else{
        for (int i = 0 ; i < strlen(theInput); i++){
				  int as_int = (int)theInput[i];
				  put_buff_1(as_int);
			}
		}
	}
    return NULL;
}

/*
	Get the next item from buffer 1
*/
int get_buff_1() {
    // Lock the mutex before checking if the buffer has data
    pthread_mutex_lock(&mutex_1);
    while (count_1 == 0)
        // Buffer is empty. Wait for the producer to signal that the buffer has data
        pthread_cond_wait(&full_1, &mutex_1);
    int item = buffer_1[con_idx_1];
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
void put_buff_2(int item) {
    // Lock the mutex before putting the item in the buffer
    pthread_mutex_lock(&mutex_2);
    // Put the item in the buffer
    buffer_2[prod_idx_2] = item;
    // Increment the index where the next item will be put.
    prod_idx_2 = prod_idx_2 + 1;
    count_2++;
    // Signal to the consumer that the buffer is no longer empty
    pthread_cond_signal(&full_2);
    // Unlock the mutex
    pthread_mutex_unlock(&mutex_2);
}

/*
	The second thread will take the values from the buf_1 and
	replace any \n it finds with spaces.
*/
void* separate_lines(void* args)
{
   int item = 0;
   while(1){
	   item = get_buff_1(); // Get the ASCII number from buf_1
     if(item == -1){
		put_buff_2(-1);
		break;
		}
	   if(item == '\n') // found newline, replace this value with 32 the space ASCII value
		  item = 32;
		// if item is not a newline either way this int get sent to buf_2
		put_buff_2(item);
	}
	return NULL;
}

/*
	Get the next item from buffer 2
*/
int get_buff_2() {
    // Lock the mutex before checking if the buffer has data
    pthread_mutex_lock(&mutex_2);
    while (count_2 == 0)
        // Buffer is empty. Wait for the producer to signal that the buffer has data
        pthread_cond_wait(&full_2, &mutex_2);
    int item = buffer_2[con_idx_2];
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
void put_buff_3(int item) {
    // Lock the mutex before putting the item in the buffer
    pthread_mutex_lock(&mutex_3);
    // Put the item in the buffer
    buffer_3[prod_idx_3] = item;
    // Increment the index where the next item will be put.
    prod_idx_3 = prod_idx_3 + 1;
    count_3++;
    // Signal to the consumer that the buffer is no longer empty
    pthread_cond_signal(&full_3);
    // Unlock the mutex
    pthread_mutex_unlock(&mutex_3);
}

/*
	The third thread will search buf_2 for ASCII number 53, or the + 
	character, if 53 is also the adjacent number then we will put ^ into buff_3
*/
void* change_input(void* args)
{
	int curItem = 0;
	//int next = 0;
	bool check; // bool to check if we have seen a ++
	while(1){
		curItem = get_buff_2();
		if(curItem == -1){
			put_buff_3(-1);
			break;
		}
		if(curItem == '+'){
			if(check == true){
				put_buff_3('^');
				check = false;
			}
			else{
				check = true;
			}
		}
		else{
			if(check == true){
			put_buff_3('+');
			}
			put_buff_3(curItem);
			check = false;
		}
    
	}
	return NULL;
}


// Get an item from buffer 3
int get_buff_3() {
    // Lock the mutex before checking if the buffer has data
    pthread_mutex_lock(&mutex_3);
    while (count_3 == 0)
        // Buffer is empty. Wait for the producer to signal that the buffer has data
        pthread_cond_wait(&full_3, &mutex_3);
    int item = buffer_3[con_idx_3];
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
 take 80 indexes from buf_3 and send it to stdout
 repeat until buf_3 has reached its end marker
*/
void* write_output(void* args)
{
	//int index = 0;
  char arrToPrint[80]; //size of greater than 80 for race conditions
	int item = 0;
  int itemCount = 0;
  int i = 0;
	while(1){ // loop forever
		item = get_buff_3();
    itemCount++;
    if(item == -1)
      break;
		arrToPrint[i] = item;
    i++;
    if(itemCount == 80){
      printf("%s\n", arrToPrint);
      itemCount = 0;
      i = 0;
    }
	}
  if(itemCount == 80){ // check if we have an 80 char line ready to print
  //  after exiting
      printf("%s\n", arrToPrint);
    }
  return NULL;
}

int main()
{
    srand(time(0));
    pthread_t input_t, line_separator_t, plus_sign_t, output_t;
    // Create the threads
    pthread_create(&input_t, NULL, get_input, NULL);
    pthread_create(&line_separator_t, NULL, separate_lines, NULL);
    pthread_create(&plus_sign_t, NULL, change_input, NULL);
    pthread_create(&output_t, NULL, write_output, NULL);

    // Wait for the threads to terminate
    pthread_join(input_t, NULL);
    pthread_join(line_separator_t, NULL);
    pthread_join(plus_sign_t, NULL);
    pthread_join(output_t, NULL);
    return EXIT_SUCCESS;
}