#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>

int main(int argc, char* argv[]){
  // Check usage & args
  if (argc < 2) {
    fprintf(stderr,"USAGE: keygen [num]\n");
    exit(0);
  }
  srand(time(NULL));
  int keyLength;
  char keyTable[27] = {'A','B','C','D','E','F','G','H','I','J','K','L','M',
  'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',' '};
  sscanf(argv[1], "%d", &keyLength); // convert or keyLength to an int
  char theKey[keyLength]; // array that will be sent to stdout
  for(int i = 0; i < keyLength; i++){ // generate random chars and place them in our key

      int index = rand() % 27; // chose a random index from keyTable
      theKey[i] = keyTable[index];
  }
  // Once our key has been generated, send to stdout
  // We can just printf
  printf("%s\n", theKey);
}
