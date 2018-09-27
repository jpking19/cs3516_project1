// (Jason King) Simple http client
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <errno.h>

// Define receive buffer size
#define RECEIVEBUFFER 3000

// Error Handling function
void DieWithError(char *errorString, int errnoNum) {
	printf("***Error***\n\n%s\n\n", errorString);
	printf("Errno # %d: %s\n", errnoNum, strerror(errnoNum));
	exit(1);
}

int main(int argc,char* argv[]) {
	int sock, returnVal, bytesReceived; // Socket descriptor
	char *serverURL, *serverPort; // Server URL and Port
	char *host, *pathPointer;
	char request[1024], response[RECEIVEBUFFER], path[1024]; // GET request and response (and path to build it)
	struct addrinfo info, *serverInfo, *results; // Used in getaddrinfo()
	// -p variables
	int pFlag = 0; // Whether or not the -p option was included (default 0)
	struct timeval start, finish;
	long RTT; // in milliseconds


	// Check for correct number of arguments
	if ((argc < 3) || (argc > 4)){
		DieWithError("Wrong number of arguments", 5);
	}

	// Assign arguments to variables (taking into account options)
	int i = argc - 3;
	if (i) {
		pFlag = 1;
	}
	serverURL = argv[i + 1];
	serverPort = argv[i + 2];

	// Split given URL into path and host
	pathPointer = strstr(serverURL, "/");
	if (!pathPointer) {
		strcpy(path, "/");
		host = serverURL;
	} else {
		strcpy(path, pathPointer);
		host = strtok(serverURL, "/");
	}

	// Create info structure (setting aside memory too)
	memset(&info, 0, sizeof(struct addrinfo)); // Zero out structure
	info.ai_family = AF_UNSPEC;
	info.ai_socktype = SOCK_STREAM;
	info.ai_flags = AI_PASSIVE;

	// Resolve URL to IP address
	if ((returnVal = getaddrinfo(host, serverPort, &info, &results)) != 0) {
		DieWithError("Could not complete 3-way handshake", errno);
	}

	// Iterate through results until we connect to something valid
	for (serverInfo = results; serverInfo != NULL; serverInfo = serverInfo->ai_next) {
		// Create socket
		if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
			printf("Could not create client socket");
			continue; // Failure, loop again
		}

		// Get start time
		gettimeofday(&start, 0);

		// Connect to server via TCP socket
		if (connect(sock, serverInfo->ai_addr, serverInfo->ai_addrlen) == -1) {
			printf("Did not connect - try again\n");
			continue; // Failure, loop again
		}

		// Get finish time
		gettimeofday(&finish, 0);

		break;
	}

	// Free memory used by results
	freeaddrinfo(results);

	// If we failed, fail
	if (serverInfo == NULL) {
		DieWithError("Not able to connect to server", errno);
	}

	// Concatenate a valid HTTP/1.1 GET request for the supplied URL
	sprintf(request, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", path, host);

	// Print get message
	printf("\nGet Request:\n%s\n", request);

	// Send GET request
	if (send(sock, request, strlen(request), 0) < 0) {
		DieWithError("Could not write to socket", errno);
	}

	// Receive response back from server
	memset(response, 0, sizeof(response));
	bytesReceived = 0;

	while((bytesReceived = read(sock, response, RECEIVEBUFFER - 1)) > 0) {
		response[bytesReceived] = 0;
		printf("%s", response); // Prints part of response from the server

		if(bytesReceived < 0) {
			DieWithError("Could not read server response", errno);
		}
	}



	// Calculate and print out RTT if required
	if (pFlag) {
		RTT = ((finish.tv_sec - start.tv_sec) * 1000000LL) + (finish.tv_usec - start.tv_usec);
		printf("\n\nRTT for accessing URL is: %ld milliseconds \n", RTT);
	}

	// Close socket
	close(sock);
    return 0;
}
