#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

int main(int argc, char* argv[]){
	// Start Time
	srand(time(NULL));
	char* key;

	if (argc != 2) {
		printf("You need more arguments");
		printf("\n");
		exit(1);
	}
	else {
		int keyLength;
		keyLength = atoi(argv[1]);
		key = (char*)malloc(sizeof(char) * (keyLength + 1));


		for (int x = 0; x < keyLength; x++) {
			key[x] = alphabet[rand() % 27];
		}

		key[keyLength] = 0;
		printf("%s\n", key);
	}
	fflush(stdout);
	return 0;
}