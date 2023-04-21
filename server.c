#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <ctype.h>
#include <string.h>

#include <sys/socket.h>

#include <netdb.h>
#include <unistd.h>    // -> write, read
#include <arpa/inet.h> // -> htons

#include "common.h"

// Function designed for chat between client and server.
void func(int sockfd, size_t buff_size) {
	char *buff = malloc(buff_size);

	forever {
		// read string from client into buffer
		bzero(buff, buff_size);
		read(sockfd, buff, buff_size);
		printf("From Client: %s\n", buff);

		// write a prompt
		printf("Send a string: ");
		fflush(stdout); // flush since we didn't write a full line

		// read string from stdin
		bzero(buff, buff_size);
		span line_s = read_line(buff, buff_size);

		// trim the string's whitespace
		span msg_s = destructive_trim((char *)line_s.ptr, buff_size);

		// send it thru the socket
		write(sockfd, msg_s.ptr, msg_s.len);

		// exit if you said to.
		if (strncmp(buff, "exit", 5) == 0) {
			printf("Server Exit...\n");
			break;
		}
	}

	free(buff);
}

int main() {
	// make a new socket
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		perror("couldn't make socket");
		exit(EXIT_FAILURE);
	}
	printf("Created socket!\n");

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
		exit(EXIT_FAILURE);
	}
	printf("Socket successfully bound!\n");

	// Now server is ready to listen and verification
	if (listen(sockfd, 5) == -1) {
		perror("couldn't listen for clients");
		close(sockfd);
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
		exit(EXIT_FAILURE);
	}
	printf("Server accepted a client!\n");

	// Function for chatting between client and server
	func(connfd, BUFF_LEN);

	// After chatting close the socket
	close(sockfd);

	exit(EXIT_SUCCESS);
}
