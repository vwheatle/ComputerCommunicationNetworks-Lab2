#
# makefile for pong_game - like pong, including walls (paddle)
#

# REPLACE THIS!!

CC = gcc
CC_FLAGS = -Wall -Wpedantic -Wextra -Werror -fsanitize=undefined
VALGRIND_FLAGS = --quiet --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=3 --error-exitcode=1

all: main
.PHONY: clean run-client run-server

main: client.c server.c common.h
	$(CC) $(CC_FLAGS) -o lab_client client.c
	$(CC) $(CC_FLAGS) -o lab_server server.c

# -c for compiling but not linking
# -g for debugging with gdb...

clean:
	rm -f ./lab_client ./lab_server

run-client:
	@valgrind $(VALGRIND_FLAGS) ./lab_client

run-server:
	@valgrind $(VALGRIND_FLAGS) ./lab_server
