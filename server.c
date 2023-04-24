#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <ctype.h>
#include <string.h>

// #include <sys/types.h>
#include <sys/socket.h>

#include <unistd.h>    // -> write, read
#include <fcntl.h>     // -> open
// #include <netdb.h>
#include <arpa/inet.h> // -> htons

#include "common.h"

// Function designed for chat between client and server.
void func(int sockfd, FILE *outfile, size_t buff_size) {
	char *buff = malloc(buff_size);

	size_t amount = 0;
	do {
		// read data from client into buffer
		amount = read(sockfd, buff, buff_size);
		printf("From Client:\n");

		// display it
		for (size_t i = 0; i < amount; fputc(buff[i++], stdout)) {}
		fputc('\n', stdout);
	} while (amount > 0);

	free(buff);
}

int main(int argc, char *argv[]) {
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

	// make a new socket
	int sockfd = socket(AF_INET, SOCK_SEQPACKET, 0);
	if (sockfd == -1) {
		perror("couldn't make socket");
		exit(EXIT_FAILURE);
	}
	printf("Created socket!\n");

	// (soon to be) adapted from this guide:
	// https://beej.us/guide/bgnet/html/#getaddrinfoprepare-to-launch

	struct sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);

	// Binding newly created socket to given IP and verification
	struct sockaddr *servaddr_p = (struct sockaddr *)&servaddr;
	if (bind(sockfd, servaddr_p, sizeof(servaddr)) == -1) {
		perror("couldn't bind socket\n");
		close(sockfd);
		fclose(outfile);
		exit(EXIT_FAILURE);
	}
	printf("Socket successfully bound!\n");

	// Now server is ready to listen and verification
	if (listen(sockfd, 5) == -1) {
		perror("couldn't listen for clients");
		close(sockfd);
		fclose(outfile);
		exit(EXIT_FAILURE);
	}
	printf("Server listening...\n");

	struct sockaddr_in clientaddr;
	socklen_t clientlen = sizeof(clientaddr);

	// Accept the data packet from client and verification
	struct sockaddr *clientaddr_p = (struct sockaddr *)&clientaddr;
	int connfd = accept(sockfd, clientaddr_p, &clientlen);
	if (connfd == -1) {
		perror("server acccept failed...\n");
		close(sockfd);
		fclose(outfile);
		exit(EXIT_FAILURE);
	}
	printf("Server accepted a client!\n");

	// Function for chatting between client and server
	func(connfd, outfile, BUFF_LEN);

	// After chatting close the socket
	close(sockfd);

	// close the output file
	fclose(outfile);

	exit(EXIT_SUCCESS);
}
