#include <string.h> // -> memset, strncmp

// things the client and server *must* agree on
// in order to work correctly.

#define BUFF_MIN 32
#define BUFF_LEN 128
#define PORT     8080

typedef struct {
	int32_t length;
	char buffer[];
} my_packet;

typedef struct {
	char message[2];
} my_response;

#define RESPONSE_OK    "OK" // "okay"
#define RESPONSE_ERROR "NG" // "No Good"
#define RESPONSE_LEAVE "BY" // "bye"

my_response make_response(char *text) {
	my_response r;
	strncpy(r.message, text, sizeof(r.message));
	return r;
}
int response_status(my_response r) {
	if (strncmp(r.message, RESPONSE_OK, sizeof(r.message)) == 0) return 0;
	if (strncmp(r.message, RESPONSE_ERROR, sizeof(r.message)) == 0) return -1;
	if (strncmp(r.message, RESPONSE_LEAVE, sizeof(r.message)) == 0) return 1;
	return -1;
}

// and some funny other stuff to share

#define as_a_string_eval(x) #x
#define as_a_string(x)      as_a_string_eval(x)

#define bzero(buff, size) memset(buff, 0, size)

#define forever for (;;)
