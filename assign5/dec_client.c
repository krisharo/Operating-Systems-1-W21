#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()
#include <stdbool.h>

#define MAX_SIZE 80000

/**
* Client code - Decoder side
* 1. Create a socket and connect to the server specified in the command arugments.
* 2. Open encrypted file and associated key file
* 3. Print the message received from the server and exit the program.
Identical to enc_client with adjustments to exclusive encoder/decoder signatures
*/

// Error function used for reporting issues
void error(const char *msg) {
  perror(msg);
  exit(0);
}

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address,
                        int portNumber,
                        char* hostname){

  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address));

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);

  // Get the DNS entry for this host name
  struct hostent* hostInfo = gethostbyname(hostname);
  if (hostInfo == NULL) {
    fprintf(stderr, "CLIENT: ERROR, no such host\n");
    exit(0);
  }
  // Copy the first IP address from the DNS entry to sin_addr.s_addr
  memcpy((char*) &address->sin_addr.s_addr,
        hostInfo->h_addr_list[0],
        hostInfo->h_length);
}

int messageSend(int socket, char buf[], int* len){ // function that will be used to send to server
  int total = 0; // number of bytes sent
  int bytesLeft = * len;
  int charsWritten;

  while(total < *len){ // continue to send until we have no more bytes left to send
    charsWritten = send(socket, buf + total, bytesLeft, 0);
    if(charsWritten == -1){
      fprintf(stderr, "CLIENT: error writing to server!\n");
      exit(2);
    }
    total += charsWritten;
    bytesLeft -= charsWritten;
  }
}

int main(int argc, char *argv[]) {  // argv layout: [enc_client, plaintext, key, port]
  int socketFD, portNumber, charsRead;
  struct sockaddr_in serverAddress;
  size_t len = 0;
  ssize_t nread;
  char buffer;
  char keyTable[27] = {'A','B','C','D','E','F','G','H','I','J','K','L','M',
  'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',' '};
  // Check usage & args
  if (argc < 3) {
    fprintf(stderr,"USAGE: %s plaintext key port\n", argv[0]);
    exit(0);
  }

  // Create a socket
  socketFD = socket(AF_INET, SOCK_STREAM, 0);
  if (socketFD < 0){
    error("CLIENT: ERROR opening socket");
  }
  char* key = malloc(MAX_SIZE * sizeof(char));
  char* plaintext = malloc(MAX_SIZE * sizeof(char));
  char ciphertext[MAX_SIZE];
  char message[2 * MAX_SIZE];

   // Set up the server address struct
  setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost");

  // Connect to server
  if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
    fprintf(stderr ,"CLIENT: ERROR connecting with server at port %d",
      ntohs(serverAddress.sin_port));
    exit(2);
  }
   // open plantext file and place into plantext[]
  FILE* theFile = fopen(argv[1], "r");
  if (theFile == NULL) {
     fprintf(stderr, "Cannot open plaintext file\n");
     exit(1);
  }
  fgets(plaintext, MAX_SIZE, theFile);
  // Remove the trailing \n that fgets adds and replace it with a terminating char
  plaintext[strcspn(plaintext, "\n")] = ',';
  fclose(theFile);
  // Check if plaintext has any bad chars
  int plainlen = strlen(plaintext) - 1; // -1 for the comma
  bool isValid = false;
  for(int i = 0; i < plainlen; i++){ //iterate through plaintext
    for(int j = 0; j < 27; j++){ // find each char in plaintext in the keytable
      if(plaintext[i] == keyTable[j]){
        isValid = true;
      } // if we found a matching char, then this plaintext char is valid
    }
    // If at any point in the plaintext do we find an invalid char, exit
    if(isValid == false){
      perror("ENC_CLIENT: Invalid character found!\n");
      exit(1);
    }
    // After checking a char, reset the value of the bool
    isValid = false;
  }

  //Repeat for key file
  theFile = fopen(argv[2], "r");
  if (theFile == NULL) {
    fprintf(stderr, "Cannot open plaintext file\n");
    exit(1);
  }
  fgets(key, MAX_SIZE, theFile);
  int charsToRead = strlen(plaintext);
   // Remove the trailing \n that fgets adds
  key[strcspn(key, "\n")] = ',';
  fclose(theFile);

  // check if keyfile is shorter than plaintext
  if(strlen(key) < strlen(plaintext)){
    fprintf(stderr, "%s is shorter than %s", argv[2], argv[1]);
    exit(1);
  }

  strcpy(message, plaintext);
  // In case key has a massive size, shrink it down to a size we only need
  char realKey[charsToRead];
  memcpy(realKey, &key[0], charsToRead - 1);
  // add the terminating character to the end of realKey
  realKey[charsToRead -1] = ',';
  realKey[charsToRead] = '\0';

  // Combine realkey and plaintext into once string which will be sent to the server
  strcpy(message, plaintext);
  strcat(message, realKey);
  strcat(message, "@"); // Terminating terminator character, enc client exclusive
  int messageLen = strlen(message);

  /** Before moving any further let me explain the commas and @ character
      We are going to send the plaintext and key as one big string to the server
      The server will keep receving from the client until it reads in the @ or ^
      Then the plaintext and key will be parsed using commas as the delimeter
      If the server finds that the wrong signature is present, then the connection
      will be forcefully terminated
  */

  // Now we're ready to write to the server
  if(messageSend(socketFD, message, &messageLen) == -1){
    fprintf(stderr, "Only %d bytes were sent to the server", messageLen);
  }

  // Now receive from the server
  // Loop until charsRead has reached charsToRead
  int count = 0;
  charsToRead = strlen(plaintext) -1;
  char buf[MAX_SIZE];
  memset(buf, '\0', MAX_SIZE);
  charsRead = recv(socketFD, buf, sizeof(buf) -1, 0);
  if(charsRead < 0){
    fprintf(stderr, "CLIENT: error receiving from server\n");
    exit(2);
  }
  if(buf[0] == '^'){ // check if we got the encoder signature
    fprintf(stderr, "ENC_CLIENT: communicated with decoder at port %d, terminating\n",
      ntohs(serverAddress.sin_port));
    exit(2);
  }
  while(charsRead != charsToRead){
    count = recv(socketFD, buf, sizeof(buf) -1, 0);
    charsRead += count;
  }
  // send ciphertext to stdout
  printf("%s\n", buf);


  // Close the socket
  close(socketFD);
  return 0;
}
