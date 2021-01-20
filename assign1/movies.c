#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> //needed to include this library to make ssize_t work

//struct for movie information
struct movie {
	char* title;
	char* year;
    char* langArr[5]; //array where each element is one language, 5 languages max
    int k;
	char* rating;
	struct movie* next;
};

//Parse csv information line-by-line
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

struct Movie* processFile(char* filePath)
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

void LangFinder(struct movie* theList, char* lang) {
    int check = 0;
    while (theList != NULL) {     
        for (int i = 0; i < theList->k; i++) {
            if (strcmp(lang, theList->langArr[i]) == 0) {
                printf("%d %s\n", atoi(theList->year), theList->title);
                check = 1;
            } 
        }
        theList = theList->next;
    }
    if (check == 0) {
        printf("No movies found for that language\n");
    }
}

int movieCounter(struct movie* theList) {
    int moviesCounted = 0;
    while (theList != NULL) {
        moviesCounted++;
        theList = theList->next;
    }
    return moviesCounted;
}

void ratingSorter(struct movie* movies, int total) {
    int years[total - 1]; //array of unique years
    for (int i = 0; i < total; i++) {
        years[i] = 0;
    }
    
    int count = 0;
    struct movie* theList = movies;
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

    for (int i = 0; i < count; i++) {
        struct movie* theList2 = movies;
        theList2 = theList2->next;
        char* theTitle = theList2->title;
        double theRating = atof(theList2->rating);
        while (theList2 != NULL) {
            if (atoi(theList2->year) == years2[i]) {
                if (atof(theList2->rating) > theRating) {
                    theTitle = theList2->title;
                    theRating = atof(theList2->rating);
                }
            }
            theList2 = theList2->next;
        }
        printf("%i %.1f %s \n", years2[i], theRating, theTitle);
    }
    

}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("You must provide the name of the file to process\n");
        printf("Example usage: ./students student_info1.txt\n");
        return EXIT_FAILURE;
    }
    struct movie* list = processFile(argv[1]);
    //printMovieList(list);
    printf("\n1. Show movies released in the specified year");
    printf("\n2. Show highest rated movie for each year");
    printf("\n3. Show the title and year of release of all movies in a specific language");
    printf("\n4. Exit from the program");
    printf("\nChose a number between 1 and 4: ");
    int choice;
    scanf("%d", &choice);
    while (choice != 4) {
        switch (choice) {
        case 1:
            printf("\nEnter the year for which you want to see movies: ");
            int year;
            scanf("%d", &year);
            printf("\nShowing movies released in %d \n", year);
            //Do search for year here
            struct movie* temp = list;
            while (temp != NULL) {
                if (atoi(temp->year) == year) {
                    printf("%s \n", temp->title);
                }
                temp = temp->next;
            }
            break;
        case 2:
            printf("\nPrinting highest rated movie of each year...\n");
            int totalMovies = movieCounter(list);
            ratingSorter(list, totalMovies);
            break;
        case 3:
            printf("\nEnter the language for which you want to see movies: ");
            char lang[20] = "";
            scanf("%s", &lang);
            //Do language search here
            LangFinder(list, lang);            
            break;
        case 4:
           //Blank as the loop will exit

        default:
            printf("\nInvalid number entered, please enter a number between 1 and 4.");

       }
        printf("\n1. Show movies released in the specified year");
        printf("\n2. Show highest rated movie for each year");
        printf("\n3. Show the title and year of release of all movies in a specific language");
        printf("\n4. Exit from the program");
        printf("\nChose a number between 1 and 4: ");
        scanf("%d", &choice);
    }

    return EXIT_SUCCESS;
}

