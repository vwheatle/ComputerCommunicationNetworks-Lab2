#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <ctype.h>
#include <string.h>

#include <sys/socket.h>

#include <unistd.h>    // -> write, read
#include <fcntl.h>     // -> open
// #include <netdb.h>
#include <arpa/inet.h> // -> htons

#include "common.h"

// Function designed for chat between client and server.
void func(int sockfd, FILE *datafile, size_t buff_size) {
	char *buff = malloc(buff_size);

	while (!feof(datafile) && !ferror(datafile)) {
		// fill the buffer with new data from the data file
		size_t num_bytes = fread(buff, 1, buff_size, datafile);

		printf("Sending %zd bytes.\n", num_bytes);

		// send it thru the socket
		write(sockfd, buff, num_bytes);
	}

	int error = ferror(datafile);
	if (error != 0) {
		fprintf(stderr, "an error occurred while reading the file.\n");
	}

	free(buff);
}

int main(int argc, char *argv[]) {
	if (argc != 2 && argc != 3) {
		char *me = argc > 0 ? argv[0] : "client";
		fprintf(stderr,
			"usage: %s in_file [buffer_size]\n"
			"if buffer size is not specified, it defaults to 128.\n",
			me);
		exit(EXIT_FAILURE);
	}

	size_t buff_size = BUFF_LEN;
	if (argc == 3) {
		// atoi doesn't have good error handling,
		// but nothing bad should happen.
		buff_size = atoi(argv[2]);
	}

	// sane minimum
	if (buff_size < BUFF_MIN) {
		buff_size = BUFF_MIN;
		fprintf(stderr,
			"given buffer size was way too small! "
			"it's %d now.\n",
			BUFF_MIN);
	}

	// open the data file before we do any socket stuff
	FILE *datafile = fopen(argv[1], "r");
	if (datafile == NULL) {
		perror("couldn't open data file");
		exit(EXIT_FAILURE);
	}

	// make a new socket
	int sockfd = socket(AF_INET, SOCK_SEQPACKET, 0);
	if (sockfd == -1) {
		perror("couldn't make socket");
		fclose(datafile);
		exit(EXIT_FAILURE);
	}
	printf("Created socket!\n");

	struct sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));

	// set up an address for the socket to use
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	int result = inet_pton(AF_INET, "127.0.0.1", &(servaddr.sin_addr));
	if (result <= 0) {
		if (result == -1) {
			perror("couldn't convert address");
		} else if (result == 0) {
			fprintf(stderr, "invalid address\n");
		}
		fclose(datafile);
		exit(EXIT_FAILURE);
	}

	// connect the client socket to server socket
	struct sockaddr *servaddr_p = (struct sockaddr *)&servaddr;
	if (connect(sockfd, servaddr_p, sizeof(servaddr)) == -1) {
		perror("couldn't connect to server");
		close(sockfd);
		fclose(datafile);
		exit(EXIT_FAILURE);
	}
	printf("Connected to server!\n");

	// function for chat.
	// takes a buffer size as its second parameter.
	func(sockfd, datafile, buff_size);

	// close the socket connection
	close(sockfd);

	// close the data file
	fclose(datafile);

	exit(EXIT_SUCCESS);
}
