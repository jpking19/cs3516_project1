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

#define QUEUE 5
#define RCVBUFSIZE 1028

int openConnection(char* port);
void DieWithErrors(char *errorMessage, int errnoNum){
  printf(errorMessage, "/n");
  printf("Errno # %d : %s/n", errnoNum, strerror(errnoNum));
  exit(EXIT_FAILURE);
}


int main(int argc, char **argv){
  int newSocket, lstnSocket = 0;
  int bytes, bytesSent;
  struct sockaddr_storage theirAddr;
  socklen_t addr_size;
  int readResult;
  char rcvBuffer[RCVBUFSIZE], rqstMethod[6], rqstPath[100], rqstHttp[10], errorMessage[50];
  FILE *file;
  long fileSize;

  //Checks to see if user gave correct number of command line arguments
  if(argc != 2)
    DieWithErrors("Usage: ./server <port>", 8);

    lstnSocket = openConnection(argv[1]);

    if(listen(lstnSocket, 5) < 0){
      DieWithErrors("Error Listening.", errno);
    }


    printf("Waiting for connection now.\n");

    while(1){
      //addrInSize = sizeof theirAddr;
      newSocket = accept(lstnSocket, (struct sockaddr *) &theirAddr, &addr_size);
      printf("%d \n", newSocket);
      if(newSocket < 0)
        DieWithErrors("Error accepting connection", errno);
      if (fork() == 0) {

        printf("Accepted Connection\n");


        memset(rcvBuffer, 0, sizeof(rcvBuffer));

        while((readResult = read(newSocket, rcvBuffer, sizeof(rcvBuffer))) > 0){
          rcvBuffer[readResult] = 0;
          printf("%s", rcvBuffer);
          if(strstr(rcvBuffer, "\r\n"))
            break;
        }

        if(readResult < 0)
          DieWithErrors("Error reading host message", errno);

          printf("\n");
          //Reading the HTTP Request Host sent
          //We know the GET message goes GET (some path) HTTP/1.1
          sscanf(rcvBuffer, "%s %s %s", rqstMethod, rqstPath, rqstHttp);
          printf("rcvBuffer is %s", rcvBuffer);

          printf("rqstMethod %s\n", rqstMethod);
          printf("rqstPath %s\n", rqstPath);
          printf("rqstHttp %s\n", rqstHttp);

          if(strcmp(rqstMethod, "GET") != 0){
            printf("yeet\n");
            strcpy(errorMessage, "Expected GET method");
            //Write to host
            if(write(newSocket, errorMessage, strlen(errorMessage)) < 0){
              close(newSocket);
              close(lstnSocket);
              DieWithErrors("Error sending error message to host", errno);
            }
            }else{
              if(strcmp(rqstPath, "/") == 0 || strcmp(rqstPath, "/index.html") == 0 || strcmp(rqstPath, "/TMDG.html") == 0){
                printf("yeet2\n");
                file = fopen("TMDG.html", "r");
                if (file == NULL){
                  close(newSocket);
                  close(lstnSocket);
                  DieWithErrors("Error opening file", 2);
                }else{
                  printf("yeet3\n");
                  //File is open and ready to transmit. Load file and send it.
                char okMessage[strlen("HTTP/1.1") + strlen("200 OK\n") + 1];
                sprintf(okMessage, "%s 200 OK\n\n", rqstHttp);
                if(write(newSocket, okMessage, strlen(okMessage)) < 0){
                  close(newSocket);
                  close(lstnSocket);
                  DieWithErrors("Error sending OK Message", errno);
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
              while(fileSize>0){
                if((bytes = write(newSocket, sendBuffer, fileSize-bytesSent)) < 0){
                  close(newSocket);
                  close(lstnSocket);
                  DieWithErrors("Error writing to socket", errno);
                }if(bytes == 0){
                  break;
                }else{
                  bytesSent+=bytes;
                  continue;
                }
              }
              free(sendBuffer);
            }
          }
        }
      }
      close(newSocket);
    }
    //We're done listening so the server will close
    close(lstnSocket);
}

  int openConnection(char* port){

    struct addrinfo myInfo;
    struct addrinfo *thisServer;
    struct addrinfo *resolvedServer;
    int listenSocket;
    int yes = 1;


    memset(&myInfo, 0, sizeof(struct addrinfo)); //we are programming in C and weird things tend to happen with regards to memory so we make sure we have enough space to allocate a packet with our information
    myInfo.ai_family = AF_UNSPEC; //we don't care whether we use IPV4 or 6
    myInfo.ai_socktype = SOCK_STREAM; //we're using TCP
    myInfo.ai_flags = AI_PASSIVE; //automatically fills in our IP

    if (getaddrinfo(NULL, port, &myInfo, &resolvedServer) != 0){
      DieWithErrors("Error connecting with Server for 3-way handshake", errno);
    }

      /* Go through all the results getaddrinfo returned until we find something valid we can connect to */
    for(thisServer = resolvedServer; thisServer != NULL; thisServer = thisServer->ai_next){
     /*First try to create a socket with the information
     socket(ipv4 or 6, stream or datagram, protocol)*/
     listenSocket = socket(thisServer->ai_family, thisServer->ai_socktype, thisServer->ai_protocol);

     if(listenSocket < 0){
       printf("Error creating client socket. Errno #%d", errno);
       continue;
     }

     //*** THIS IS WHERE NEW SERVER CODE STARTS***
     //This takes a port and reuses it for this program in case that port is busy
     if(setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
       DieWithErrors("Error reusing a socket.", errno);

     if(bind(listenSocket, thisServer->ai_addr, thisServer->ai_addrlen) < 0){
       printf("Binding Error. Errno # %d\n", errno);
       close(listenSocket);
       continue;
     }
     break;
   }

   if(thisServer == NULL)
     DieWithErrors("Error binding.", errno);
   printf("The listen socket is %d\n", listenSocket);

   return listenSocket;
}
