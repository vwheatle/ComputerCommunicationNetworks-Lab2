#include <stdlib.h>  // -> EXIT_*
#include <stdio.h>   // -> printf, fopen, ...
#include <stdbool.h> // -> bool

#include <sys/types.h>
#include <sys/socket.h>

#include <unistd.h>    // -> write, read
#include <fcntl.h>     // -> open
#include <netdb.h>     // -> getaddrinfo
#include <arpa/inet.h> // -> htons

#include "common.h"

// Function designed for chat between client and server.
void func(int sockfd, FILE *outfile, size_t buff_size) {
	size_t packet_size = sizeof(my_packet) + buff_size;
	my_packet *packet = malloc(packet_size);

	size_t num_bytes = 0;
	do {
		// read data from client into buffer
		num_bytes = read(sockfd, packet, packet_size);
		if (num_bytes <= sizeof(my_packet)) break;

		// convert the length back into a usable number
		packet->length = ntohl(packet->length);
		if (packet->length > buff_size) packet->length = (uint32_t)buff_size;

		printf("Receiving %zd-byte buffer from Client.\n",
			num_bytes - sizeof(my_packet));

		// display it
		for (size_t i = 0; i < packet->length; i++)
			fputc(packet->buffer[i], stdout);
		fputc('\n', stdout);

		// write it into the destination file
		fwrite(packet->buffer, sizeof(char), packet->length, outfile);
	} while (packet->length >= buff_size);

	free(packet);
}

int main(int argc, char *argv[]) {
	// The Boilerplate

	if (argc != 2 && argc != 3) {
		char *me = argc > 0 ? argv[0] : "server";
		fprintf(stderr,
			"usage: %s out_file [buffer_size]\n"
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
		fprintf(stderr, "given buffer size was way too small! it's %d now.\n",
			BUFF_MIN);
	}

	// create the output file before we do any socket stuff
	FILE *outfile = fopen(argv[1], "w");
	if (outfile == NULL) {
		perror("couldn't create output file");
		exit(EXIT_FAILURE);
	}

	// The Networking Stuff

	// adapted from this guide:
	// https://beej.us/guide/bgnet/html/#getaddrinfoprepare-to-launch

	// give getaddrinfo some hints on what we want from the helper object
	struct addrinfo hints;
	bzero(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;      // don't care if IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM;  // must be SOCK_STREAM to accept()
	hints.ai_flags = AI_PASSIVE;      // like INADDR_ANY
	hints.ai_flags |= AI_NUMERICSERV; // the service will be a port number

	// make the fancy getaddrinfo helper object!
	struct addrinfo *svinfo;
	int gai_status = getaddrinfo(NULL, as_a_string(PORT), &hints, &svinfo);
	if (gai_status != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(gai_status));
		fclose(outfile);
		exit(EXIT_FAILURE);
	}

	// make a new socket from the address info
	int sockfd =
		socket(svinfo->ai_family, svinfo->ai_socktype, svinfo->ai_protocol);
	if (sockfd == -1) {
		perror("couldn't make socket");
		freeaddrinfo(svinfo);
		fclose(outfile);
		exit(EXIT_FAILURE);
	}
	printf("Created socket!\n");

	// binding our new socket to the given IP
	if (bind(sockfd, svinfo->ai_addr, svinfo->ai_addrlen) == -1) {
		perror("couldn't bind socket");
		close(sockfd);
		freeaddrinfo(svinfo);
		fclose(outfile);
		exit(EXIT_FAILURE);
	}
	printf("Socket successfully bound!\n");

	// start listening for clients
	if (listen(sockfd, 5) == -1) {
		perror("couldn't listen for clients");
		close(sockfd);
		freeaddrinfo(svinfo);
		fclose(outfile);
		exit(EXIT_FAILURE);
	}
	printf("Server listening...\n");

	struct sockaddr_in clientaddr;
	socklen_t clientlen = sizeof(clientaddr);

	// accept the data packet from client and verification
	struct sockaddr *clientaddr_p = (struct sockaddr *)&clientaddr;
	int connfd = accept(sockfd, clientaddr_p, &clientlen);
	if (connfd == -1) {
		perror("server acccept failed");
		close(sockfd);
		freeaddrinfo(svinfo);
		fclose(outfile);
		exit(EXIT_FAILURE);
	}
	printf("Server accepted a client!\n");

	// thing to do once we know we have established a server-client connection.
	func(connfd, outfile, BUFF_LEN);

	// Cleanup

	// close the socket
	close(sockfd);

	// free the fancy linked-list address info object
	freeaddrinfo(svinfo);

	// close the output file
	fclose(outfile);

	exit(EXIT_SUCCESS);
}
