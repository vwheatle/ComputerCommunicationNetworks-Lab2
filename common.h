#include <stdlib.h>  // -> size_t
#include <stdbool.h> // -> bool

#include <ctype.h>  // -> isspace
#include <string.h> // -> strnlen

// things the client and server *must* agree on
// in order to work correctly.

#define BUFF_LEN 128
#define PORT     8080

// and some funny other stuff to share

#define bzero(buff, size) memset(buff, 0, size)

#define forever for (;;)

typedef struct {
	void *ptr;
	size_t len;
} span;

span read_line(char *buff, size_t buff_size) {
	size_t len = 0;
	for (; len < buff_size - 1; len++) {
		char ch = fgetc(stdin);
		if (ch < 0x20) break;
		buff[len] = ch;
	}
	buff[len] = '\0';
	return (span) {buff, len};
}

span destructive_trim(char *buff, size_t buff_size) {
	size_t last = strnlen(buff, buff_size) - 1;
	while (last > 0 && isspace(buff[last])) buff[last--] = '\0';

	size_t len = last + 1;
	while (len > 0 && isspace(buff[0])) {
		buff[0] = '\0';
		buff = &buff[1];
		len--;
	}

	return (span) {buff, len};
}
