#include <stdio.h>   // -> std*
#include <stdlib.h>  // -> size_t
#include <stdbool.h> // -> bool

#include <ctype.h>  // -> isspace
#include <string.h> // -> strnlen

// things the client and server *must* agree on
// in order to work correctly.

#define BUFF_MIN 32
#define BUFF_LEN 128
#define PORT     8080

// and some funny other stuff to share

#define bzero(buff, size) memset(buff, 0, size)

#define forever for (;;)
