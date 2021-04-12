#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h> 
#include <fcntl.h>
#include <sys/wait.h>
#include <stdbool.h>

const int maxChars = 2048;
const int maxArgs = 512;
bool mode = true; // global bool for SIGSTP

struct cmdPrompt { //struct for parsing input
    char* args[512];
    char* inputFile;
    char* outputFile;
    int pid;
    bool comment;
    bool background;
    bool input;
    bool output;
};

void cleanUp(struct cmdPrompt* APrompt) { // will reset the strings of the struct to not mess up the next inputs
    memset(APrompt->args, '\0', sizeof(APrompt->args));
    APrompt->comment = false;
    APrompt->background = false;
    if (APrompt->input) {
        free(APrompt->inputFile);
        APrompt->inputFile = NULL;
    }
    if (APrompt->output) {
        free(APrompt->outputFile);
        APrompt->outputFile = NULL;
    }
    APrompt->pid = 0;
    APrompt->input = false;
    APrompt->output = false;
}

char* replaceWith(const char* orig, const char* dollar, const char* thePid) { // Will return string with the $$ being replaced by pid
    char* result; // string with expanded input
    int i, count = 0;
    int len1 = strlen(thePid);
    int len2 = strlen(dollar);

    for (i = 0; orig[i] != '\0'; i++) { // check how often we find $$
        if (strstr(&orig[i], dollar) == &orig[i]) {
            count++;
            i += len1 - 1;
        }
    }
    result = (char*)malloc(i + count * (len1 - len2) + 1);

    i = 0;
    while (*orig) {
        // compare the substring with the result 
        if (strstr(orig, dollar) == orig) {
            strcpy(&result[i], thePid);
            i += len1;
            orig += len2;
        }
        else
            result[i++] = *orig++;
    }

    result[i] = '\0';
    return result;
}

struct cmdPrompt* getInput(int PID) { // will get input from stdin, similar to how input was parsed in assign1
	char input[maxChars];
	fgets(input, maxChars, stdin);
    char stringPid[5];
    sprintf(stringPid, "%d", PID);
    char* temp = replaceWith(input, "$$", stringPid);
    strcpy(input, temp);

	struct cmdPrompt* aPrompt = malloc(sizeof(struct cmdPrompt));
	aPrompt->pid = PID;
	if (input[0] == '\n' || input[0] == '#' || input[0] == ' ') { //will return an empty struct that does nothing
		aPrompt->comment = true;
		return aPrompt;
	}
	bool once = false;
	for (int i = 0; !once && i < maxChars + 1; i++) { // get rid of newline char
		if (input[i] == '\n') {
			input[i] = '\0'; 
			once = true;
		}
	}
    aPrompt->comment = false;
	char* saveptr = NULL;
	char* token = strtok_r(input, " ", &saveptr); // extract tokens with " " being the delim
	int argCount = 0;
	while (token != NULL && strcmp(token, "\0")) {
		if ((strcmp(token, "&")) == 0) { // Background process, though bool mode has to be true for it to execute in the background
			aPrompt->background = 1;
			strtok_r(NULL, " ", &saveptr);
		}
		else if (strcmp(token, "<") == 0) { // found an input file
			token = strtok_r(NULL, " ", &saveptr);
			aPrompt->inputFile = strdup(token);
            aPrompt->input = true;
		}
		else if (strcmp(token, ">") == 0) { // found an output file
			token = strtok_r(NULL, " ", &saveptr);
			aPrompt->outputFile = strdup(token);
            aPrompt->output = true;
		}
		else { // at this point there is nothing special about the string extracted, just the command followed by args
            aPrompt->args[argCount] = strdup(token);
            argCount++;
		}
		token = strtok_r(NULL, " ", &saveptr);
	}
	
	return aPrompt;
}

void handleSIGTSTP(int signo) {
    // Turn foreground-only mode on/off
    if (mode == true) { 
        char* message = "Entering foreground-only mode (& is now ignored)\n";
        write(1, message, 49);
        fflush(stdout);
        mode = false;
    }

    // If it's 0, set it to 1 and display a message reentrantly
    else {
        char* message = "Exiting foreground-only mode\n";
        write(1, message, 29);
        fflush(stdout);
        mode = false;
    }
}

void getExitStatus(int status) { // Determines if the process ended "naturally" or killed by a signal
   
    if (WIFEXITED(status)) {
        printf("exit value %d\n", WEXITSTATUS(status));
    }
    else
        printf("terminated by signal %d\n", WTERMSIG(status));
}

int main() {
    int pid = getpid();
    int exitStatus = 0;
    int loophandle = 1;
    char input[2048]; //where the entire line of user input will go
    int children[200];
    int childrenCount = 0; // number of background processes

    // Signal handlers, from signal handling api module
    // Ignore CTRL + C
    struct sigaction SIGINT_action = { 0 };
    SIGINT_action.sa_handler = SIG_IGN;
    sigfillset(&SIGINT_action.sa_mask);
	SIGINT_action.sa_flags = 0;
    sigaction(SIGINT, &SIGINT_action, NULL);

    // Create an interrupt to toggle foreground-only mode
    struct sigaction SIGTSTP_action = { 0 };
    SIGTSTP_action.sa_handler = handleSIGTSTP;
    sigfillset(&SIGTSTP_action.sa_mask);
	SIGTSTP_action.sa_flags = 0;
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);

    do { //loop forever
        printf(": ");
        fflush(stdout);
		struct cmdPrompt* thePrompt  = getInput(pid); // Will parse input into our struct   

        //Now check for special commands
        if (thePrompt->comment == true) {}
        else if (strcmp(thePrompt->args[0], "exit") == 0) {
            //Kill any other processes or jobs, then exit the program
            int i = 0, pid;
            for (i = 0; i < 200; i++) {
                if (pid == 0) { // if parent just exit 
                    exit(0);
                }
                kill(children[i], SIGTERM); // kill the pid    
            }
        }
        else if (strcmp(thePrompt->args[0], "cd") == 0) {
            if (thePrompt->args[1] == '\0') {
                // go to root directory
                chdir(getenv("HOME"));
            }
            else {
                //change directory to the directory specified by args[1]
                chdir(thePrompt->args[1]);
            }
        }
        else if (strcmp(thePrompt->args[0], "status") == 0) {
			getExitStatus(exitStatus);
        }
        else {
            pid_t spawnpid = -5;
            int result, sourceFD, targetFD = 0;
            // If fork is successful, the value of spawnpid will be 0 in the child, the child's pid in the parent
            spawnpid = fork();
            switch (spawnpid) { //From fork example replit
            case -1:
                // Code in this branch will be exected by the parent when fork() fails and the creation of child process fails as well
                perror("fork() failed!");
                exit(1);
                break;

            case 0:
                // spawnpid is 0. This means the child will execute the code in this branch
                // Handle CTRL + C
				SIGINT_action.sa_handler = SIG_DFL;
				sigaction(SIGINT, &SIGINT_action, NULL);
                
                //Handle input & output files
                if (thePrompt->input) { // Based on sortViaFiles replit
                     sourceFD = open(thePrompt->inputFile, O_RDONLY);
                    if (sourceFD == -1) {
                        perror("source open()");
                        exit(1);
                    }
                    // Redirect stdin to source file
                    result = dup2(sourceFD, 0);

                    if (result == -1) {
                        perror("source dup2()");
                        exit(2);
                    }
                    // close the file
                    close(sourceFD);
                }

                if (thePrompt->output) {
                    //Open target file
                    targetFD = open(thePrompt->outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (targetFD == -1) {
                        perror("target open()");
                        exit(1);
                    }
                    result = dup2(targetFD, 1);
                    if (result == -1) {
                        perror("target dup2()");
                        exit(1);
                    }
                    close(targetFD);
                }
				// Execute it
                execvp(thePrompt->args[0], thePrompt->args);
                perror("execv");
                if (result == 0)
                    fflush(stdout);
                exit(1);
                break;

            default:
                if (thePrompt->background && mode) { // Check for background processes
                    // Print the child's pid
                    printf("Child's pid = %d\n", spawnpid);
                    fflush(stdout);
                    // Keep track of pid by storing it in children[]
                    children[childrenCount] = spawnpid;
                    childrenCount++;
                    
                }
                else {
                    if (!mode)
                        spawnpid = waitpid(-1, &exitStatus, 0);
                    else
                        spawnpid = waitpid(spawnpid, &exitStatus, 0);
                }
                break;
            }
        }
        cleanUp(thePrompt); // clean up the struct
        // check for terminated processes
        int childpid = 0;
        childpid = waitpid(-1, &exitStatus, WNOHANG);
        while (childpid > 0) {
			 printf("Background process %d has finished ", childpid);
            getExitStatus(exitStatus);
            fflush(stdout);
            for (int i = 0; i < childrenCount; i++) {
                if (children[i] == childpid) {
                    children[i] = -5;
                    childrenCount--;
                }
            }
            childpid = waitpid(-1, &exitStatus, WNOHANG);
        } // restart the loop
    } while (loophandle);

}
