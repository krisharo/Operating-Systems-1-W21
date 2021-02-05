#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/types.h>
#include<time.h>
#include <fcntl.h>
#include <unistd.h>

#define PREFIX "movies_"

struct movie {
	char* title;
	char* year;
    char* langArr[5]; //array where each element is one language, 5 languages max
    int k;
	char* rating;
	struct movie* next;
};

struct movie* createMovie(char* curLine) {
	struct movie* curMovie = malloc(sizeof(struct movie));
	char* saveptr;
	
	//Parse title
	char* token = strtok_r(curLine, ",", &saveptr);
	curMovie->title = calloc(strlen(token) + 1, sizeof(char));
	strcpy(curMovie->title, token);

	//year
	token = strtok_r(NULL, ",", &saveptr);
	curMovie->year = calloc(strlen(token) + 1, sizeof(char));
	strcpy(curMovie->year, token);

	//languages
    int i = 0;
    saveptr++;
    //while loop to search for semicolons in the case of multiple languages
    while (strchr(saveptr, ';') != NULL) {
        token = strtok_r(NULL, ";", &saveptr);
        curMovie->langArr[i] = calloc(strlen(token) + 1, sizeof(char));
        strcpy(curMovie->langArr[i], token);
        i++;
        curMovie->k++;
    }

    //will need to copy again for the last language (or if just one language)
    token = strtok_r(NULL, ",", &saveptr);
    //before allocating memory, get rid of "]"
    token[strlen(token) - 1] = '\0';
    curMovie->langArr[i] = calloc(strlen(token) + 1, sizeof(char));
    strcpy(curMovie->langArr[i], token);
    curMovie->k++;
    
	//rating
	token = strtok_r(NULL, "\n", &saveptr);
	curMovie->rating = calloc(strlen(token) + 1, sizeof(char));
	strcpy(curMovie->rating, token);

	curMovie->next = NULL;
	return curMovie;
}

struct Movie* processCSV(char* filePath)
{
    FILE* movies = fopen(filePath, "r");
    if (movies == NULL) {
        perror("File failed to open");
    }

    char* currLine = NULL;
    size_t len = 0;
    ssize_t nread;
    //char* token;

    // The head of the linked list
    struct movie* head = NULL;
    // The tail of the linked list
    struct movie* tail = NULL;

    int moviecount = 0;
    // Read the file line by line
    while ((nread = getline(&currLine, &len, movies)) != -1)
    {
        // Get a new movie node corresponding to the current line
        struct movie* newNode = createMovie(currLine);
        

        // Is this the first node in the linked list?
        if (head == NULL)
        {
            // This is the first node in the linked link
            // Set the head and the tail to this node
            head = newNode;
            tail = newNode;
        }
        else
        {
            // This is not the first node.
            // Add this node to the list and advance the tail
            tail->next = newNode;
            tail = newNode;
        }
        moviecount++;
    }
    printf("Processed the file ");
    printf("%s ", filePath);
    printf("and parsed data for ");
    printf("%d ", moviecount);
    printf("movies \n");
    free(currLine);
    fclose(movies);
    return head;
}

int movieCounter(struct movie* theList) {
    int moviesCounted = 0;
    while (theList != NULL) {
        moviesCounted++;
        theList = theList->next;
    }
    return moviesCounted;
}

/**
This function will:
	Print the file being processed (DONE)
	Create a new directory (DONE)
	Print the name of the created directory in the format "onid.movies.random" (DONE)
	random is a random number 0-99999 inclusive (DONE)
	Set directory permissions to rwxr-x--- (read, write & execute) (DONE)
	Parse data in chosen file to find out the movies released in each year
	In the new directory, create one file for each year in which at least one movie was released.
		File permissions are rw-r-----
		File must be named YYYY.txt
		Movie titles released in that year will be in the file, one title per line.
*/
void processFile(char* fileName){
	printf("Now processing the file %s", fileName);
	//Create a new directory
	srand(time(NULL));
  int r = rand() % 100000; //random number used for directory #
  printf("%d\n", r);
  char* onid = "harokr.movies.";
  char rng[6];
  sprintf(rng, "%d", r);
  int len = strlen(onid) + 6;
  char dirName[len];
  strcpy(dirName,onid);
  strcat(dirName,rng);
  printf("\n%s", dirName);
  mkdir(filename, S_IRWXU | S_IRGRP | S_IXGRP); //Directory made
  //Parse csv data
  struct movie* list = processCSV(fileName);
  //Get an array of unique years
  int total = movieCounter(list);
  int years[total - 1]; //array of unique years
    for (int i = 0; i < total; i++) {
        years[i] = 0;
    }
    
    int count = 0;
    struct movie* theList = list;
    theList = theList->next; //ignore first line
    while (theList != NULL) { //start at first movie
        int check = 1;
        for (int i = 0; i < total -1; i++) {
            if (atoi(theList->year) == years[i]) {
                check = 0;
            }
        }
        if (check == 1) {
            years[count] = atoi(theList->year);
            count++;
        }
        theList = theList->next; //next movie->year
    }
    
    int years2[count];
    //Print array of unique years
    for (int i = 0; i < count; i++) {
        years2[i] = years[i];
	}
  /** For each index in the array, iterate through the list, creating a file
  for the movies with matching years
  */
  for (int i = 0; i < count; i++) {
		//Create file here
		int file_descriptor;
		char curYear[4];
		sprintf(curYear, "%d", years2[i]);
		char ext[5] = ".txt\0";
		char* slash = "/";
		len = strlen(filename) + 10; //10 for length of year, .txt, and slash
		char newFilePath[len];
		strcpy(newFilePath, filename);
		strcat(newFilePath, slash);
		strcat(newFilePath, year);
		strcat(newFilePath, ext);
		//Using the opening/closing file example
		file_descriptor = open(newFilePath, O_RDWR | O_CREAT, 0640); //File has been made
		//Now iterate through the list finding appropirate movies
        struct movie* theList2 = movies;
        theList2 = theList2->next;
        //char* theTitle = theList2->title;
        while (theList2 != NULL) {
            if (atoi(theList2->year) == years2[i]) {
                //write to file here
				write(file_descriptor, theList2->title, strlen(theList2->title));
            }
            theList2 = theList2->next;
        }
    }
	//After iterating through the list, close the file
	
}

//The functions findSmallest and findLargest are based off the "Getting File and Directory Meta-Data"
// examples
char* findSmallest(){
	DIR* currDir = opendir(".");
  struct dirent *aDir;
  struct stat dirStat;
  char myChar[256];
  int size;
  int minSize = 0;
  char smallest[256];
  static bool once = false;
    
  while((aDir = readdir(currDir)) != NULL){

    if(strncmp(PREFIX, aDir->d_name, strlen(PREFIX)) == 0){ //found file with suitable prefix
      // Get meta-data for the current entry
      stat(aDir->d_name, &dirStat);  
      strcpy(myChar,aDir->d_name);
      char* ext = strrchr(myChar, '.');
      if(ext != NULL){
        if(strcmp(ext, ".txt") == 0){
          printf("Suitable file found\n");
          //Now get its size
          size = dirStat.st_size;
          printf("%s, size %d bytes\n", myChar, size);
          //We want the first file to be the smallest, so we initialize min value only once
          if(once == false){
            minSize = size;
            strcpy(smallest,myChar);
            once = true;
          }
          if(size < minSize){
            minSize = size;
            strcpy(smallest,myChar);
          }
          }
        }
      }
      
      
  }
  closedir(aDir);
  printf("\n The smallest file in the directory is %s, with size %d bytes\n",
  smallest, minSize);
  return smallest;
}

char* findLargest(){
	DIR* currDir = opendir(".");
  struct dirent *aDir;
  struct stat dirStat;
  char myChar[256];
  int size;
  int maxSize = 0;
  char largest[256];
    
  while((aDir = readdir(currDir)) != NULL){

    if(strncmp(PREFIX, aDir->d_name, strlen(PREFIX)) == 0){ //found file with suitable prefix
      // Get meta-data for the current entry
      stat(aDir->d_name, &dirStat);  
      strcpy(myChar,aDir->d_name);
      char* ext = strrchr(myChar, '.');
      if(ext != NULL){
        if(strcmp(ext, ".csv") == 0){
          printf("Suitable file found\n");
          //Now get its size
          size = dirStat.st_size;
          printf("%s, size %d bytes\n", myChar, size);
          if(size > maxSize){
            maxSize = size;
            strcpy(largest,myChar);
          }
          }
        }
      }
      
      
  }
  closedir(aDir);
  printf("\n The largest file in the directory is %s, with size %d bytes\n",
  largest, maxSize);
  return largest;
}

int main()
{
    printf("\n1. Select file to process");
    printf("\n2. Exit the program");
    printf("\nEnter a choice 1 or 2: ");
    int choice;
    scanf("%d", &choice);
    while (choice != 2) {
        switch (choice) {
        case 1:
			label:
            printf("\nWhich file you want to process?");
			printf("\nEnter 1 to pick the largest file");
			printf("\nEnter 2 to pick the smallest file");
			printf("\nEnter 3 to specify the name of a file");
			int choice2;
			scanf("%d", &choice2);
			switch(choice2){
				case 1: //Find largest file
				char* file = findLargest();
				processFile(file);
				break;
				
				case 2: //Find smallest file
				char* file = findSmallest;
				processFile(file);
				break;
				
				case 3: //Specify the name of a file
				printf("\nEnter the namne of the file: ");
				char* name;
				scanf("%s", &name);
				//Seach name in the directory if it exists
				int file = -1;
				file = open(name, O_RDWR); //From the Opening & Closing exploration code
				if(file == -1){
					printf("\nThe file %s was not found. Try again", name);
					goto label;
				}
				else{
					
				} 
				break;
				
				default:
					printf("\nInvalid number entered, please enter a number between 1 and 3.");
			}
            break;
        case 2:

        default:
            printf("\nInvalid number entered, please enter a number between 1 and 2.");

       }
        printf("\n1. Select file to process");
		printf("\n2. Exit the program");
		printf("\nEnter a choice 1 or 2: ");
        scanf("%d", &choice);
    }

    return EXIT_SUCCESS;
}