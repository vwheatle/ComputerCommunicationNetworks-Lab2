#include <string.h> // -> memset

// things the client and server *must* agree on
// in order to work correctly.

#define BUFF_MIN 32
#define BUFF_LEN 128
#define PORT     8080

typedef struct {
	uint32_t length;
	char buffer[];
} my_packet;

// and some funny other stuff to share

#define as_a_string_eval(x) #x
#define as_a_string(x)      as_a_string_eval(x)

#define bzero(buff, size) memset(buff, 0, size)

#define forever for (;;)
