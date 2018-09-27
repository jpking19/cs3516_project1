// (Jason King) Simple http server
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>

// Define max queue size
#define QUEUE 5
// Define receive buffer size
#define RECEIVEBUFFER 1028

int openConnection(char* port);

// Error Handling function
void DieWithErrors(char *errorMessage, int errnoNum){
  printf(errorMessage, "/n");
  printf("Errno # %d : %s/n", errnoNum, strerror(errnoNum));
  exit(EXIT_FAILURE);
}


int main(int argc, char **argv) {
  int newSock, listenSock = 0; // Socket descriptors
  int bytes, bytesSent, readResult;
  struct sockaddr_storage theirAddr;
  socklen_t addr_size;
  char rcvBuffer[RECEIVEBUFFER], rqstMethod[6], rqstPath[100], rqstHttp[10], errorMessage[50];
  FILE *file;
  long fileSize;

  // Check for correct number of arguments
  if (argc != 2) {
        DieWithErrors("Wrong number of arguments", 5);
  }

  listenSock = openConnection(argv[1]);

  if(listen(listenSock, 5) < 0){
    DieWithErrors("Error Listening.", errno);
  }

  printf("Waiting for connection now.\n");

  // Enter infinite loop
  while (1) {
    // Accept the next connection
    newSock = accept(listenSock, (struct sockaddr *) &theirAddr, &addr_size);

    printf("%d \n", newSock);

    if (newSock < 0) {
      DieWithErrors("Could not accept connection", errno);
    }

    // Start new process
    if (fork() == 0) {
      printf("Accepted Connection\n");

      memset(rcvBuffer, 0, sizeof(rcvBuffer));

      while ((readResult = read(newSock, rcvBuffer, sizeof(rcvBuffer))) > 0) {
        rcvBuffer[readResult] = 0;
        printf("%s", rcvBuffer);

        // Break when you hit "\r\n"
        if (strstr(rcvBuffer, "\r\n")) {
          break;
        }
      }

      if (readResult < 0) {
        DieWithErrors("Could not read host request", errno);
      }

      // Read the request from the host
      sscanf(rcvBuffer, "%s %s %s", rqstMethod, rqstPath, rqstHttp);
      // Ensure method is GET (we dont support anything else)
      if (strcmp(rqstMethod, "GET") != 0) {
        strcpy(errorMessage, "Expected GET method");

        // Notify host
        if (write(newSock, errorMessage, strlen(errorMessage)) < 0) {
          close(newSock);
          close(listenSock);

          DieWithErrors("Incorrectly formatted request", errno);
        }
      } else {
        // Ensure the host has specified a valid path
        if(strcmp(rqstPath, "/") == 0 || strcmp(rqstPath, "/index.html") == 0 || strcmp(rqstPath, "/TMDG.html") == 0) {
          file = fopen("TMDG.html", "r");
          // Open the file
          if (file == NULL){
            close(newSock);
            close(listenSock);

            DieWithErrors("Could not open file", 2);
          } else {
            // Load file and send to host
            char okMessage[strlen("HTTP/1.1") + strlen("200 OK\n") + 1];
            sprintf(okMessage, "%s 200 OK\n\n", rqstHttp);
            if (write(newSock, okMessage, strlen(okMessage)) < 0) {
              close(newSock);
              close(listenSock);

              DieWithErrors("Could not send OK Message", errno);
            }

            //Read the file before sending it out
            fseek(file, 0, SEEK_END);
            fileSize= ftell(file);
            fseek(file, 0, SEEK_SET);

            char *sendBuffer = malloc(fileSize * sizeof(char) + 1);
            memset(sendBuffer, 0, fileSize);

            fread(sendBuffer, fileSize, 1, file);
            fclose(file);
            sendBuffer[fileSize] = 0;

            bytesSent = 0;
            while (fileSize>0) {
              if ((bytes = write(newSock, sendBuffer, fileSize-bytesSent)) < 0) {
                close(newSock);
                close(listenSock);

                DieWithErrors("Could not write to socket", errno);
              }
              if (bytes == 0) {
                break;
              } else {
                bytesSent+=bytes;
                continue;
              }
            }
            free(sendBuffer);
          }
        }
      }
    }
    close(newSock);
  }
  // Close server
  close(listenSock);
}

int openConnection(char* port) {
  struct addrinfo info;
  struct addrinfo *serverInfo;
  struct addrinfo *results;
  int listenSock, returnVal;
  int yes = 1;

  // Create info structure (setting aside memory too)
	memset(&info, 0, sizeof(struct addrinfo)); // Zero out structure
	info.ai_family = AF_UNSPEC;
	info.ai_socktype = SOCK_STREAM;
	info.ai_flags = AI_PASSIVE;

  // Resolve URL to IP address
	if ((returnVal = getaddrinfo(NULL, port, &info, &results)) != 0) {
		DieWithErrors("Could not complete 3-way handshake", errno);
	}

  // Iterate through results until we connect to something valid
  for (serverInfo = results; serverInfo != NULL; serverInfo = serverInfo->ai_next) {
     // Create socket
     listenSock = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

     if ((listenSock = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol)) < 0) {
       printf("Could not create client socket");
 			 continue; // Failure, loop again
     }

     if (setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
       DieWithErrors("Could not reuse a socket", errno);
     }

     if (bind(listenSock, serverInfo->ai_addr, serverInfo->ai_addrlen) < 0) {
       printf("Could not bind");
       close(listenSock);
       continue;
     }
     break;
   }

   if (serverInfo == NULL) {
     DieWithErrors("Error binding.", errno);
   }
   printf("The listen socket is %d\n", listenSock);

   return listenSock;
}
