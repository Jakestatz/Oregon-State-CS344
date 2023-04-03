#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Error function used for reporting issues
void error(const char* message) {
	perror(message);
	exit(1);
}

void setupAddressStruct(struct sockaddr_in* address, int portnumber) {
	// Clear out the address struct
	memset((char*)address, '\0', sizeof(*address));

	// The address should be network capable
	address->sin_family = AF_INET;
	address->sin_port = htons(portnumber);
	address->sin_addr.s_addr = INADDR_ANY;
}

void encrypt(char msg[], char key[]) {
	char encrypted;

	int x;
	for (x = 0; msg[x] != '\n'; x++) {
		if (msg[x] != ' ') {
			msg[x] = msg[x] - 65;
		}
		else {
			msg[x] = 26;
		}

		if (key[x] != ' ') {
			key[x] = key[x] - 65;
		}
		else {
			key[x] = 26;
		}

		encrypted = key[x] + msg[x];
		encrypted = encrypted % (26 + 1);

		if (encrypted != 26) {
			msg[x] = encrypted + 65;
		}

		else {
			msg[x] = ' ';
		}
	}
	msg[x] = '\0';
}

int main(int argc, char* argv[]) {
	int connectionSocket;
	char buffer[100000];
	struct sockaddr_in serverAddress, clientAddress;
	socklen_t sizeOfClientInfo = sizeof(clientAddress);

	// Check usage & args
	if (argc < 2) {
		fprintf(stderr, "USAGE: %s port\n", argv[0]);
		exit(1);
	}

	// Create the socket that will listen for connections
	int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket < 0) {
		error("ERROR opening socket");
	}
	// Set up the address struct for the server socket
	setupAddressStruct(&serverAddress, atoi(argv[1]));

	// Associate the socket to the port
	if (bind(listenSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
		error("ERROR on binding");
	}

	// Start listening for connetions. Allow up to 5 connections to queue up
	listen(listenSocket, 5);

	// Accept a connection, blocking if one is not available until one connects
	while (1) {
		// Accept the connection request which creates a connection socket
		connectionSocket = accept(listenSocket, (struct sockaddr*)&clientAddress, &sizeOfClientInfo);

		if (connectionSocket < 0) {
			error("ERROR on accept");
		}

		int pid = fork();

		if (pid == 0) {
			int buffersize = sizeof(buffer);

			for (int x = 0; x < buffersize; x++) {
				buffer[x] = '\0';
			}

			read(connectionSocket, buffer, buffersize - 1);
			int check = strcmp(buffer, "yes1");

			if (check == 0) {
				write(connectionSocket, "yes1", 5);
			}

			else {
				exit(2);
			}

			buffersize = sizeof(buffer);

			for (int y = 0; y < buffersize; y++) {
				buffer[y] = '\0';
			}

			char* placeholder = buffer;
			char* start;
			int linecount, remaining;
			int first = 1;
			int messagelength, messagesize;

			while (1) {
				if (first == 1) {
					remaining = buffersize;
					first = first + 1;
					linecount = 0;
				}

				int completed = read(connectionSocket, placeholder, remaining);

				if (completed == 0) {
					exit(1);
				}

				for (int z = 0; z < completed; z++) {
					if (placeholder[z] == '\n') {
						linecount = linecount + 1;

						if (linecount != 1) {
							continue;
						}

						else {
							start = placeholder + z + 1;
						}
					}
				}

				placeholder = placeholder + completed;
				remaining = remaining - completed;

				if (linecount == 2) {
					break;
				}
				else {
					continue;
				}
			}

			messagelength = start - buffer;
			char message[messagelength];
			messagesize = sizeof(message);

			for (int a = 0; a < messagesize; a++) {
				message[a] = '\0';
			}

			strncpy(message, buffer, messagelength);
			encrypt(message, start);
			write(connectionSocket, message, messagesize);

		}

		else if (pid == -1) {
			printf("ERROR on forking");
			exit(1);
		}

		else {
			close(connectionSocket);

			int terminate = 0;

			do {
				waitpid(pid, &terminate, 0);
			} while (!WIFEXITED(terminate) && !WIFSIGNALED(terminate));
		}
	}

	close(listenSocket);
	return 0;
}