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

		// read string from server into buffer
		bzero(buff, buff_size);
		read(sockfd, buff, buff_size);
		printf("From Server: %s\n", buff);

		// exit if the server said to.
		if (strncmp(buff, "exit", 5) == 0) {
			printf("Client Exit...\n");
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
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(PORT);

	// connect the client socket to server socket
	struct sockaddr *servaddr_p = (struct sockaddr *)&servaddr;
	if (connect(sockfd, servaddr_p, sizeof(servaddr)) == -1) {
		perror("couldn't connect to server");
		close(sockfd);
		exit(EXIT_FAILURE);
	}
	printf("Connected to server!\n");

	// function for chat.
	// takes a buffer size as its second parameter.
	func(sockfd, BUFF_LEN);

	// close the socket connection
	close(sockfd);

	exit(EXIT_SUCCESS);
}
