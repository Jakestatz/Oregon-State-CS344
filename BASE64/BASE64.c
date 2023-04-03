#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <err.h>

#define WRAP 76
// Check that uint8_t type exists
#ifndef UINT8_MAX
#error "No support for uint8_t"
#endif
static char const alphabet[] =	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

size_t encode(uint8_t* in, size_t length, size_t count) {
	uint8_t out[4];
	out[0] = in[0] >> 2;
	out[1] = (in[0] << 4 | in[1] >> 4) & 0x3F;
	out[2] = (in[1] << 2 | in[2] >> 6) & 0x3F;
	out[3] = in[2] & 0x3F;

	if (length < 3) {
		out[3] = 64;
		out[2] &= 0x3C;
	}
	if (length < 2) {
		out[2] = 64;
		out[1] &= 0x30;
	}
	for (size_t i = 0; i < sizeof out / sizeof* out; ++i) {
		if (putchar(alphabet[out[i]]) == EOF) {
			err(errno, "putchar()");
		}
		if (++count == WRAP) {
			if (putchar('\n') == EOF) {
				err(errno, "putchar()");
				count = 0;
			}
		}
	}
	return count;
}

int main(int argc, char* argv[]) {
	FILE* Fname = stdin;
	if (argc == 1 || strcmp("-", argv[1]) == 0) {

	}
	else if (argc == 2) {
		if (strcmp("-", argv[1])) {
			Fname = fopen(argv[1], "r");
			if (Fname == NULL) {
				err(errno, "fopen()");
			}
		}
		else {
			err(errno = EINVAL, "Usage: %s [FILE]", argv[0]);
		}
	}
	else {
		err(errno = EINVAL, "Too many arguments");
	}
	size_t count = 0;
	while (1) {
		uint8_t in[3];
		size_t length = fread(in, sizeof * in, sizeof in / sizeof * in, Fname);
		if (length < sizeof in / sizeof * in && ferror(Fname)) {
			err(errno, "fread()");
		}
		if (length == 0 && feof(Fname)) {
			break;
		}
		count = encode(in, length, count);
		if (length < sizeof in / sizeof * in && feof(Fname)) {
			break;
		}
	}
	if (count != 0 && putchar('\n') == EOF) {
		err(errno, "puthcar()");
	}
	if (fflush(stdout) == EOF) {
		err(errno, "fflush()");
	}
	if (Fname != stdin && fclose(Fname) == EOF) {
		err(errno, "fclose()");
	}
	return EXIT_SUCCESS;
}