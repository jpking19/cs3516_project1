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
#define RECEIVEBUFFER 4000

// Error Handling function
void DieWithError(char *errorString) {
	printf("***Error***\n\n%s\n\n", errorString);
	printf("Errno # %d: %s\n", errno, strerror(errno));
	exit(1);
}

int main(int argc,char* argv[])
{
	int sock, returnVal; // Socket descriptor
	char *serverURL, *serverPort; // Server URL and Port
	char *path, *host;
	char request[1024], response[RECEIVEBUFFER]; // GET request and response
	struct addrinfo info, *serverInfo, *results; // Used in getaddrinfo()
	// -p variables
	int pFlag = 0; // Whether or not the -p option was included (default 0)
	struct timeval start, finish;
	long RTT; // in milliseconds


	// Check for correct number of arguments
	if ((argc < 3) || (argc > 4)){
		DieWithError("Wrong number of arguments");
	}

	// Assign arguments to variables (taking into account options)
	int i = argc - 3;
	if (i) {
		pFlag = 1;
	}
	serverURL = argv[i + 1];
	serverPort = argv[i + 2];

	// Create info structure (setting aside memory too)
	memset(&info, 0, sizeof(struct addrinfo)); // Zero out structure
	info.ai_family = AF_UNSPEC;
	info.ai_socktype = SOCK_STREAM;
	info.ai_flags = AI_PASSIVE;

	// Resolve URL to IP address
	if ((returnVal = getaddrinfo(serverURL, serverPort, &info, &results)) != 0) {
		DieWithError("Could not complete 3-way handshake");
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
			printf("Could not connect to server");
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
		DieWithError("Not able to connect to server");
	}

	// Split given URL into path and host
	path = strstr(serverURL, "/");
	if (path == NULL) {
		path = "/";
		host = serverURL;
	} else {
		host = strtok(serverURL, "/");
	}

	// Concatenate a valid HTTP/1.1 GET request for the supplied URL
	sprintf(request, "GET %s \r\nHost: %s\r\n\r\n", path, host);

	// Send GET request
	if (send(sock, request, strlen(request), 0) < 0) {
		DieWithError("Could not write to socket");
	}

	// Receive response back from server
	//TODO possibly fix this
	int byte_count = recv(sock, &response, sizeof(response), 0);
	response[byte_count] = 0; // need to add the null terminator

	//TODO -p option
	if (pFlag) {
		RTT = ((finish.tv_sec - start.tv_sec) * 1000000LL) + (finish.tv_usec - start.tv_usec);
		printf("RTT for accessing URL is: %ld milliseconds \n\n", RTT);
	}

	//TODO write this to a file for submission and print out
	printf("%s\n\n", response);

	// Close socket
	close(sock);
    return 0;
}
