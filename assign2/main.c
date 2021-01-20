#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/types.h>

/**
This function will:
	Print the file being processed
	Create a new directory
	Print the name of the created directory in the format "onid.movies.random"
	random is a random number 0-99999 inclusive
	Set directory permissions to rwxr-x--- (read, write & execute)
	Parse data in chosen file to find out the movies released in each year
	In the new directory, create one file for each year in which at least one movie was released.
		File permissions are rw-r-----
		File must be named YYYY.txt
		Movie titles released in that year will be in the file, one title per line.
*/
void processFile(char* fileName){
	
}

char* findSmallest(){
	
}

char* findLargest(){
	DIR* currDir = opendir(".");
	if((currDir = opendir(".")) == NULL) {
           fprintf(stderr, "cannot open directory");
           return;
        }
  struct dirent *aDir;
  struct stat st;
  char* name;
  
  // Go through all the entries
   while((aDir = readdir(currDir)) != NULL){
	   
  }
  // Close the directory
  closedir(currDir);
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