#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

/**
* Client code
* 1. Create a socket and connect to the server specified in the command arugments.
* 2. Prompt the user for input and send that input as a message to the server.
* 3. Print the message received from the server and exit the program.
*/

// Error function used for reporting issues
void error(const char* message) {
	perror(message);
	exit(0);
}

int validate(char* filename, int filesize) {
	char buffer[filesize];
	int copy = open(filename, 'r');
	const char* alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
	for (int x = 0; x < filesize; x++) {
		int check = read(copy, buffer, 1);

		if (check == 0) {
			exit(1);
		}
		else {
			if ((buffer[0] >= 'A' && buffer[0] <= 'Z') || (buffer[0] == ' ') || (buffer[0] == '\n')) {
				continue;
			}
			else {
				return 1;
			}
		}
	}
	return 0;
}
int filevalidator(char* filename, int filesize) {
	char buffer[filesize];
	int copy = open(filename, 'r');
	const char* alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
	int x;

	for (x = 0; x < filesize; x++) {
		int check = read(copy, buffer, 1);

		if (check == 0) {
			exit(1);
		}

		else {
			if ((buffer[0] >= 'A' && buffer[0] <= 'Z') || (buffer[0] == ' ') || (buffer[0] == '\n')) {
				continue;
			}

			else {
				fprintf(stderr, "%s contains invalid characters\n", filename);
				exit(2);
			}
		}
	}
}

void sendfiletoserver(char* file, int connectionsocket, int filelength) {
	char buffer[100000];
	int buffersize = sizeof(buffer);

	int a;
	for (a = 0; a < buffersize; a++) {
		buffer[a] = '\0';
	}

	int copy = open(file, 'r');

	int x;
	int remaining;

	for (x = 0; x < filelength; x++) {
		remaining = read(copy, buffer, buffersize);

		if (remaining == 0) {
			exit(1);
		}

		filelength = filelength - remaining;
	}

	char* placeholder = buffer;
	int completed = 0;

	int y;

	for (y = 0; y < remaining; y++) {
		remaining = remaining - completed;
		placeholder = placeholder + completed;
		completed = write(connectionsocket, placeholder, remaining);
	}

	return;
}

void setupAddressStruct(struct sockaddr_in* address, int portnumber, char* hostname) {
	// Clear out the address struct
	memset((char*)address, '\0', sizeof(*address));

	// The address should be network capable
	address->sin_family = AF_INET;
	address->sin_port = htons(portnumber);

	// Get the DNS entry for this host name
	struct hostent* hostinfo = gethostbyname(hostname);

	if (hostinfo == NULL) {
		printf("CLIENT: ERROR, no such host\n");
		exit(0);
	}

	// Copy the first IP address from the DNS entry to sin_addr.s_addr
	memcpy((char*)&address->sin_addr.s_addr, hostinfo->h_addr_list[0], hostinfo->h_length);
}

int main(int argc, char* argv[]) {
	int socketFD;
	struct sockaddr_in serverAddress;
	char buffer[100000];
	char hostname[] = "localhost";
	// Check usage & args
	if (argc < 3) {
		fprintf(stderr, "USAGE: %s hostname port\n", argv[0]);
		exit(0);
	}

	// Create a socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0);

	if (socketFD < 0) {
		error("CLIENT: ERROR opening socket");
	}

	// Set up the server address struct
	setupAddressStruct(&serverAddress, atoi(argv[3]), hostname);

	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
	{
		error("CLIENT: ERROR connecting");
	}
	else
	{
		write(socketFD, "yes1", 5);
	}

	int buffersize = sizeof(buffer);

	read(socketFD, buffer, sizeof(buffer));
	int check = strcmp(buffer, "yes1");

	if (check != 0)
	{
		exit(2);
	}

	else
	{
		FILE* file1 = fopen(argv[1], "r");

		if (file1 == NULL)
		{
			error("ERROR file not found");
		}

		fseek(file1, 0L, SEEK_END);
		long int file1size = ftell(file1);
		fclose(file1);

		filevalidator(argv[1], file1size);

		FILE* file2 = fopen(argv[2], "r");

		if (file2 == NULL)
		{
			error("ERROR file not found");
		}

		fseek(file2, 0L, SEEK_END);
		long int file2size = ftell(file2);
		fclose(file2);

		if (file2size <= file1size) {
			printf("ERROR the key is too short\n");
			exit(1);
		}

		else {
			int a;
			for (a = 0; a < buffersize; a++) {
				buffer[a] = '\0';
			}

			sendfiletoserver(argv[1], socketFD, file1size);
			sendfiletoserver(argv[2], socketFD, file2size);
			int x = read(socketFD, buffer, buffersize - 1);

			if (x < 0) {
				printf("Client: ERROR reading from socket");
				exit(1);
			}

			printf("%s\n", buffer);
			close(socketFD);
			return 0;
		}
	}
}