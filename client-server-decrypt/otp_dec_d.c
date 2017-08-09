/* * * * * * * * * * * * * * * * * * * * * * * * * *
 * Name: Jacob Wilson
 * Course: CS344_400
 * Due Date: 12/2/16
 * Description: Program acts as the encryption server
 *  (background daemon). It interacts with a client
 *  and encrypts message sent from the client.
 * References in otp_dec.c
 *  * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

void sendFile(char*, int);
void receiveFile(char*, int);
char* decryptMessage(char*, char*);

int main(int argc, char *argv[]) {
  int portNumber, sockFD, newSockFD, handshake, pid;

  // various buffers hold message/key/any other c-string
  char *decryptedBuffer = malloc(sizeof(char) * 200000);
  char messageBuffer[200000];
  char keyBuffer[200000];

  // networking structs from CS344 lectures
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t clientLength;

  // check for correct arguments
  if (argc < 2) {
    fprintf(stderr, "USAGE: %s <port>\n", argv[0]);
    exit(1);
  };

  // set up server struct
  memset((char *)&serverAddress, '\0', sizeof(serverAddress));
  portNumber = atoi(argv[1]);
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(portNumber);
  serverAddress.sin_addr.s_addr = INADDR_ANY;

  // create socket
  sockFD = socket(AF_INET, SOCK_STREAM, 0);
  if (sockFD < 0) {
    perror("ERROR opening socket\n");
    exit(1);
  };

  // bind socket
  if (bind(sockFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
    perror("ERROR binding socket\n");
    exit(1);
  };

  // begin listening on the socket, up to 5 connections
  listen(sockFD, 5);

  // determine size of client address
  clientLength = sizeof(clientAddress);

  while (1) {
    // accept each new connection
    newSockFD = accept(sockFD, (struct sockaddr *)&clientAddress, &clientLength);
    if (newSockFD < 0) {
      perror("ERROR: connection not accepted\n");
    };

    // create child process
    pid = fork();

    // send/receive with each connection in a child process
    if (pid == 0) {
      close(sockFD);

      // perform handshake with client
      handshake = write(newSockFD, "def", 3);

      // receive the message
      receiveFile(messageBuffer, newSockFD);

      // receive the key
      receiveFile(keyBuffer, newSockFD);

      // decrypt the message
      memset(decryptedBuffer, 0, sizeof(decryptedBuffer));
      decryptedBuffer = decryptMessage(messageBuffer, keyBuffer);

      // send decrypted message to the client
      sendFile(decryptedBuffer, newSockFD);
      exit(0);
    } else if (pid < 0) {
      perror("ERROR: fork failed\n");
    };
    close(newSockFD);
  };
  close(sockFD);
  return 0;
};

/* * * * * * * * * * * * * * * * * * * *
 * Name: receiveFile
 * Parameters: file to receive as char
 *  array and socket as int
 * Description: loops to receive a file
 *  over the provided socket.
 *  * * * * * * * * * * * * * * * * * */
void receiveFile(char *message, int sockFD) {
  // variables to track bytes received on each attempt, and the cumulative received
  int receivedBytes;
  int receivedTotal = 0;

  // loop receiving file data until null is reached
  do {
    // track the number of bytes received from each recv attemp
    receivedBytes = recv(sockFD, message, 200000, 0);

    // if a recv attempt fails, reset byte counter to avoid loss of data
    if (receivedBytes < 0) {
      receivedBytes = 0;
    };

    // accumulate the total bytes received from each attempt
    receivedTotal += receivedBytes;
  } while (message[receivedTotal] != '\0');

  // send a receipt confirmation to the client
  receivedBytes = write(sockFD, "file received", 13);
};

/* * * * * * * * * * * * * * * * * * * * *
 * Name: sendFile
 * Parameters: file to send as char array
 *  and sock as int
 * Description: loops calling send to send
 *  a file over a specified socket
 *  * * * * * * * * * * * * * * * * * * */
void sendFile(char *message, int sockFD) {
  // variables to track bytes sent
  int sentBytes, messageSize;
  int sentTotal = 0;

  messageSize = strlen(message);

  // send the message over the passed in socket
  while (sentTotal < messageSize) {
    sentBytes = send(sockFD, message, (messageSize - sentTotal), 0);

    // if the send fails at any ponit, reset bytes to zero
    if (sentBytes < 0) {
      sentBytes = 0;
    };

    // accumulate the total sent
    sentTotal += sentBytes;
  };
};

/* * * * * * * * * * * * * * * * * * * * * *
 * Name: decryptMessage
 * Parameters: message to decrypt as char
 *  array and key to use for decryption
 *  as char array.
 * Description: Performs modulo 27 addition
 *  to decrypted message using a plaintest
 *  file and key.
 *  * * * * * * * * * * * * * * * * * * * */
char *decryptMessage(char *message, char *key) {
  int length, messageChar, keyChar, decryptChar;
  int i = 0;

  length = strlen(message);
  char *decryption = malloc(sizeof(char) * length);

  // loop through characters in message to decrypt
  while (message[i] != '\n') {
    // mark spaces with asterisks
    if (message[i] == ' ') {
      message[i] = '@';
    };
    if (key[i] == ' ') {
      key[i] = '@';
    };

    // typecast each char of the message and key to int and perform
    // ASCII calculation to get numbers into 0-27 range
    messageChar = (int) message[i];
    keyChar = (int) key[i];
    messageChar = messageChar - 64;
    keyChar = keyChar - 64;

    // calculate decrypted character
    decryptChar = messageChar - keyChar;

    // if value is negative, loop back through alphabet
    if (decryptChar < 0) {
      decryptChar += 27;
    };

    // convert back to capital character
    decryptChar += 64;

    // convert back to char
    decryption[i] = (char) decryptChar + 0;

    // return marked asterisks to spaces
    if (decryption[i] == '@') {
      decryption[i] = ' ';
    };
    i++;
  };
  return decryption;
};
