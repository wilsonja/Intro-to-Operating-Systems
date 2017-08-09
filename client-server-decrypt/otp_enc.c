/* * * * * * * * * * * * * * * * * * * * * * * * *
 * Name: Jacob Wilson
 * Course: CS344_400
 * Due Date: 12/2/16
 * Description: Program acts as foreground client
 *  in encryption process. Communicates with
 *  encryption server.
 * References in otp_dec.c
 *  * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>

void validateFile(char*, int);
void sendFile(char*, int);
void receiveFile(char*, int);

int main(int argc, char *argv[]) {
  int sockFD, portNumber, fileptr, messageSize, keySize, confirm;

  // various buffers hold message/key/any other c-string
  char messageBuffer[200000];
  char keyBuffer[200000];
  char buffer[200000];

  // networking structs from CS344 lectures
  struct sockaddr_in serverAddress;
  struct hostent *serverHost;

  // check for correct arguments
  if (argc < 4) {
    fprintf(stderr, "USAGE: %s <file> <key> <port>\n", argv[0]);
    exit(0);
  };

  // set up server struct per example from CS344 assignment description
  memset((char*)&serverAddress, '\0', sizeof(serverAddress));
  portNumber = atoi(argv[3]);
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(portNumber);
  serverHost = gethostbyname("localhost");
  if (serverHost == NULL) {
    fprintf(stderr, "CLIENT: ERROR, host unavailable\n");
    exit(0);
  };
  memcpy((char*)&serverAddress.sin_addr, (char*)serverHost->h_addr, serverHost->h_length);

  // set up socket
  sockFD = socket(AF_INET, SOCK_STREAM, 0);
  if (sockFD < 0) {
    fprintf(stderr, "ERROR opening socket\n");
    exit(1);
  };

  // connect to server over socket
  if (connect(sockFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
    perror("CLIENT: ERROR connecting\n");
    exit(1);
  };

  // open message for inspection
  fileptr = open(argv[1], O_RDONLY);
  if (fileptr < 0) {
    perror("ERROR: unable to open message\n");
    exit(0);
  };

  // determine size of message
  messageSize = read(fileptr, messageBuffer, sizeof(messageBuffer));

  // check contents of message file
  validateFile(messageBuffer, messageSize);
  close(fileptr);

  // open key for inspection
  fileptr = open(argv[2], O_RDONLY);
  if (fileptr < 0) {
    perror("ERROR: unable to open key\n");
    exit(0);
  };

  // determine size of key
  keySize = read(fileptr, keyBuffer, sizeof(keyBuffer));

  // check contents of key file
  validateFile(keyBuffer, keySize);
  close(fileptr);

  // compare key and message sizes
  if (keySize < messageSize) {
    perror("ERROR: key size too small\n");
    exit(1);
  };

  // perform handshake, read in confirmation from server
  memset(buffer, 0, sizeof(buffer));
  confirm = read(sockFD, buffer, 3);

  // compare handshake message with expected message
  if (strcmp(buffer, "abc") != 0) {
    perror("ERROR: handshake failed, cannot connect to otp_dec_d\n");
    exit(1);
  };

  // send message for encryption and wait for confirmation from server
  sendFile(argv[1], sockFD);
  memset(messageBuffer, 0, sizeof(messageBuffer));
  confirm = read(sockFD, messageBuffer, sizeof(messageBuffer));

  // send key to server and wit for receipt confirmation from server
  sendFile(argv[2], sockFD);
  memset(keyBuffer, 0, sizeof(keyBuffer));
  confirm = read(sockFD, keyBuffer, sizeof(keyBuffer));

  // receive and print encrypted message
  memset(buffer, 0, sizeof(buffer));
  receiveFile(buffer, sockFD);
  printf("%s\n", buffer);

  close(sockFD);
  return 0;
};

/* * * * * * * * * * * * * * * * * * * * * 
 * Name: validateFile
 * Parameters: char array and int for size
 * Description: Function loops through the
 *  file provided and checks for invalid
 *  characters.
 *  * * * * * * * * * * * * * * * * * * */
void validateFile(char *file, int size) {
  int i;

  // loop through the message and check for bad charaxcters
  for (i = 0; i < size - 1; i++) {
    if (file[i] > 90 || (file[i] < 65 && file[i] != 32)) {
      perror("ERROR: file characters not allowed\n");
      exit(0);
    };
  };
};

/* * * * * * * * * * * * * * * * * * * * *
 * Name: sendFile
 * Parameters: message to send as char array
 *  and socket as int.
 * Description: sends a passed in message
 *  over a specified socket.
 *  * * * * * * * * * * * * * * * * * * * */
void sendFile(char *message, int sockFD) {
  // variables to track bytes sent
  int sentBytes, messageSize;
  int sentTotal = 0;

  // new buffer stores passed in file
  char buffer[200000];

  // file pointer for passed in file
  FILE *fileptr;

  // open the mssage and check for error
  fileptr = fopen(message, "r");
  if (fileptr < 0) {
    perror("ERROR: unable to open message\n");
    exit(0);
  };

  // store the message into the buffer
  memset(buffer, 0, sizeof(buffer));
  fgets(buffer, sizeof(buffer), fileptr);
  messageSize = strlen(buffer);

  // send the message over the pased in socket
  while (sentTotal < messageSize) {
    sentBytes = send(sockFD, buffer, (messageSize - sentTotal), 0);

    // if the send fails at any point, reset bytes to zero
    if (sentBytes < 0) {
      sentBytes = 0;
    };
    sentTotal += sentBytes;
  };
  fclose(fileptr);
};

/* * * * * * * * * * * * * * * * * * * * * * * *
 * Name: receiveFile
 * Parameters: message to receive as char array
 *  and socket as int
 * Description: sends a file over a provided
 *  socket.
 *  * * * * * * * * * * * * * * * * * * * * * * */
void receiveFile(char *message, int sockFD) {
  // variables to track bytes received on each attempt, and the cumulative received
  int receivedBytes;
  int receivedTotal = 0;

  // loop receiving file data until null is reached
  do {
    // track the number of bytes received from each recv attempt
    receivedBytes = recv(sockFD, message, 200000, 0);

    // if a recv attempt fails, reset byte counter to avoid loss of data
    if (receivedBytes < 0) {
      receivedBytes = 0;
    };

    // accumulate the total bytes received from each attempt
    receivedTotal += receivedBytes;
  } while (message[receivedTotal] != '\0');
};
