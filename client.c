#include <stdlib.h>  // -> EXIT_*
#include <stdio.h>   // -> printf, fopen, ...
#include <stdbool.h> // -> bool

#include <sys/types.h> // -> size_t, ssize_t
#include <sys/socket.h>

#include <unistd.h>    // -> write, read
#include <fcntl.h>     // -> open
#include <netdb.h>     // -> getaddrinfo
#include <arpa/inet.h> // -> htons

#include "common.h"

// Function designed for chat between client and server.
void func(int sockfd, FILE *datafile, size_t buff_size) {
	size_t packet_size = sizeof(my_packet) + buff_size;
	my_packet *packet = malloc(packet_size);
	bzero(packet, packet_size);

	my_response response;

	size_t num_bytes;
	while (!feof(datafile) && !ferror(datafile)) {
		read(sockfd, &response, sizeof(response));
		if (response_status(response) != 0) break;

		// fill the buffer with new data from the data file
		num_bytes = fread(packet->buffer, sizeof(char), buff_size, datafile);

		// add some metadata telling how many bytes the content takes up.
		packet->length = htonl((uint32_t)num_bytes);

		printf("Sending %zd-byte packet.\n", num_bytes + sizeof(my_packet));

		// send data thru the socket
		write(sockfd, packet, packet_size);
	}

	int error = ferror(datafile);
	if (error != 0) {
		fprintf(stderr, "an error occurred while reading the file.\n");
		packet->length = -1;
		write(sockfd, packet, packet_size);
	} else if (response_status(response) == 0) {
		// tell server there's no more to read if it didn't already quit
		// (for example, if the file fits in an integer number of packets)
		packet->length = 0;
		write(sockfd, packet, packet_size);
	}
	// in either of these cases, you don't listen for the response.

	free(packet);
}

int main(int argc, char *argv[]) {
	// Simple 2000: The Boilerplate

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

	// The Networking Stuff

	// adapted from this guide:
	// https://beej.us/guide/bgnet/html/#getaddrinfoprepare-to-launch

	// give getaddrinfo some hints on what we want from the helper object
	struct addrinfo hints;
	bzero(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;      // don't care if IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM;  // must be SOCK_STREAM to be accept()ed
	hints.ai_flags |= AI_NUMERICHOST; // the host will be an address
	hints.ai_flags |= AI_NUMERICSERV; // the service will be a port number

	// make the fancy getaddrinfo helper object!
	struct addrinfo *svinfo;
	int gai_status =
		getaddrinfo("127.0.0.1", as_a_string(PORT), &hints, &svinfo);
	if (gai_status != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(gai_status));
		fclose(datafile);
		exit(EXIT_FAILURE);
	}

	// make a new socket
	int sockfd =
		socket(svinfo->ai_family, svinfo->ai_socktype, svinfo->ai_protocol);
	if (sockfd == -1) {
		perror("couldn't make socket");
		freeaddrinfo(svinfo);
		fclose(datafile);
		exit(EXIT_FAILURE);
	}
	printf("Created socket!\n");

	// connect the client socket to server socket
	if (connect(sockfd, svinfo->ai_addr, svinfo->ai_addrlen) == -1) {
		perror("couldn't connect to server");
		close(sockfd);
		freeaddrinfo(svinfo);
		fclose(datafile);
		exit(EXIT_FAILURE);
	}
	printf("Connected to server!\n");

	// thing to do once we know we have established a server-client connection.
	func(sockfd, datafile, buff_size);

	// Cleanup

	// close the socket connection
	close(sockfd);

	// free the fancy linked-list address info object
	freeaddrinfo(svinfo);

	// close the data file
	fclose(datafile);

	exit(EXIT_SUCCESS);
}
