#
# makefile for lab 2
#

CC = gcc
CC_FLAGS = -Wall -Wpedantic -Wextra -fsanitize=undefined
VALGRIND_FLAGS = --quiet --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=3 --error-exitcode=1

all: main
.PHONY: clean run-client run-server

main: client.c server.c common.h
	$(CC) $(CC_FLAGS) -g -o lab_client client.c
	$(CC) $(CC_FLAGS) -g -o lab_server server.c

# -c for compiling and waiting until later to link
# -g for debugging symbols, to use with gdb or valgrind

clean:
	rm -f ./lab_client ./lab_server

run-client:
	valgrind $(VALGRIND_FLAGS) ./lab_client ./beemovie.txt

run-server:
	valgrind $(VALGRIND_FLAGS) ./lab_server ./beemovie-recv.txt
	diff -qs ./beemovie.txt ./beemovie-recv.txt
