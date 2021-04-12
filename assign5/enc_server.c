#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>

#define MAX_SIZE 80000

// Error function used for reporting issues
void error(const char *msg) {
  perror(msg);
  exit(1);
}

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address,
                        int portNumber){

  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address));

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);
  // Allow a client at any address to connect to this server
  address->sin_addr.s_addr = INADDR_ANY;
}

/*
  This function will take the buffer received from the client (plaintext) and convert it to
  ciphertext
  Take the message, add key to it
  Added a modulo helper function in decoder so it also here for consistency
*/
int modulo(int x,int N){
    return (x % N + N) %N;
}

char* ciphertext(char arr[], char keyArr[], char theTable[]){
  int len = strlen(arr);
  char* result = (char*)calloc(1, len);
  char temp[len];
  for(int i = 0; i <len; i++){
    // map the ascii value of the char to an index on keyTable
    int tableIndex1 = keyArr[i] - 65;
    int tableIndex2 = arr[i] - 65;
    // include special cases for space character
    if(keyArr[i] == ' '){
      tableIndex1 = 26;
    }
    if(arr[i] == ' '){
      tableIndex2 = 26;
    }
    // Sum the indexes and encrypt
    int total = tableIndex1 + tableIndex2;
    total = modulo(total, 27);
    temp[i] = theTable[total];
  }

  // Now that we're done iterating, we can copy temp to arr
  strcpy(result, temp);
  return result;
}

int messageSend(int socket, char buf[], int* len){ // function that will be used to send to client
  int total = 0; // number of bytes sent
  int bytesLeft = * len;
  int charsWritten;

  while(total < *len){ // continue to send until we have no more bytes left to send
    charsWritten = send(socket, buf + total, bytesLeft, 0);
    if(charsWritten == -1){
      fprintf(stderr, "SERVER: error writing to client!\n");
      // exit(2);
    }
    total += charsWritten;
    bytesLeft -= charsWritten;
  }
}


int main(int argc, char *argv[]){ // syntax: enc_server listenting_port
  int connectionSocket, charsRead, charsWritten;
  char buffer[MAX_SIZE * 2];
  char plaintext[MAX_SIZE];
  char key[MAX_SIZE];
  char *encText = malloc(MAX_SIZE * sizeof(char));
  char keyTable[27] = {'A','B','C','D','E','F','G','H','I','J','K','L','M',
  'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',' '};
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);

  // Check usage & args
  if (argc < 2) {
    fprintf(stderr,"USAGE: %s port\n", argv[0]);
    exit(1);
  }

  // Create the socket that will listen for connections
  int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (listenSocket < 0) {
    error("ERROR opening socket");
  }
  else
   printf("socket opened\n");

  // Set up the address struct for the server socket
  setupAddressStruct(&serverAddress, atoi(argv[1]));

  // Associate the socket to the port
  if (bind(listenSocket,
          (struct sockaddr *)&serverAddress,
          sizeof(serverAddress)) < 0){
    error("ERROR on binding");
  }
  printf("listening for connections\n");

  // Start listening for connetions. Allow up to 5 connections to queue up
  listen(listenSocket, 5);

  // Accept a connection, blocking if one is not available until one connects
  while(1){


    // Accept the connection request which creates a connection socket
    connectionSocket = accept(listenSocket, (struct sockaddr *)&clientAddress,
                &sizeOfClientInfo);


    /* At this point we have accepted and have generated a socket for communication
      where we will achieve the following:
      * fork a process
      * the process will check if the server is connected to the client
      * Get the plaintext/key from the client via socket
      * Write back the ciphertext to the client
    */

    // Start by forking, code from smallsh assignment
    pid_t spawnpid = -5;
    int result = 0;
    spawnpid = fork();
    switch(spawnpid){
      case -1:
        perror("fork() failed!");
        exit(1);
      break;

      case 0: // for successful forking we will execute child processes here
        // Check if the server is connected to the client
        if (connectionSocket < 0){
          error("ERROR on accept");
        }
        else{
          printf("SERVER: Connected to client running at host %d port %d\n",
                                ntohs(clientAddress.sin_addr.s_addr),
                                ntohs(clientAddress.sin_port));
          // get plaintext from client
          int i = 0;
          memset(buffer, '\0', MAX_SIZE * 2);
          bool hasEnded = false;
          charsRead = recv(connectionSocket, buffer, (MAX_SIZE * 2) -1, 0);
          if(charsRead < 0){
            fprintf(stderr, "SERVER: Error reading from socket");
          }
          // check if we got the entirety of the message
          for(int i = 0; i < strlen(buffer); i++){
            if(buffer[i] == '^'|| buffer[i] == '@'){
              hasEnded = true;
            }
          }
          // If we didn't receive the whole message, continue to receive in smaller dumps
          if(hasEnded == false){
            char tempBuffer[10];
            int extra = 0;
            while(hasEnded != true){
              memset(tempBuffer, '\0', 10);
              extra = recv(connectionSocket, tempBuffer, 9, 0);
              for(int i = 0; i < strlen(tempBuffer); i++){
                if(tempBuffer[i] == '^' || tempBuffer[i] == '@'){
                  hasEnded = true;
                }
              }
              strcat(buffer, tempBuffer); // add the remaining text to the master buffer
            }
          }
          // The message we received from the client is one large string with the text and key
          // Now parse through buffer, logic from assign1
          char* saveptr;
          char* token = strtok_r(buffer, ",", &saveptr);
          strcpy(plaintext, token);
          token = strtok_r(NULL, ",", &saveptr);
          strcpy(key, token);
          // check if we communicating with the wrong client
          if(saveptr[0] != '^'){
            fprintf(stderr, "ENC_SERVER port %d: message not from enc_client!\n",
              ntohs(clientAddress.sin_port));
            // send enc exclusive char back to client and close the connection
            char* temp = "^";
            int tempLen = strlen(temp);
            send(connectionSocket, temp, tempLen, 0);
            close(connectionSocket);
            exit(0);
            break;
          }

          // At this point we can encrypt our text
          encText = ciphertext(plaintext, key, keyTable);
          int messageLen = strlen(encText);
          if(messageSend(connectionSocket, encText, &messageLen) == -1){
            fprintf(stderr, "Only %d bytes were sent to the client\n", messageLen);
          }
          // Close the connection socket for this client now that we're done
          close(connectionSocket);
        }
      break;
    }

    // At the end of the loop, we'll restart to keep listening for new connections
  }
  // Close the listening socket
  close(listenSocket);
  return 0;
}
