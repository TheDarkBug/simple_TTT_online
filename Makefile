SRC = .
CC = gcc
NAME = Simple_TTT
PLATFORM = $(shell uname)
ifeq ($(PLATFORM), Linux)
	CFLAGS = -O3
	LDFLAGS = -L $(SRC)/lib/raylib -l:libraylib.a -lpthread -lm -ldl
else ifeq ($(PLATFORM), windows32)
	CFLAGS = -Wl,--subsystem,windows
	LDFLAGS = -L $(SRC)/lib/raylib -l:libraylib.a -lopengl32 -lwinmm -lgdi32 -static -lwinpthread -lwsock32
else ifeq ($(PLATFORM), linux_win)
	CC = x86_64-w64-mingw32-gcc
	CFLAGS = -Wl,--subsystem,windows
	LDFLAGS = -L $(SRC)/lib/raylib -l:libraylib.a -lopengl32 -lwinmm -lgdi32 -static -lwinpthread -lwsock32
endif
.PHONY: server

build: main client gui shapes gameplay server
	$(CC) $(CFLAGS) -o $(NAME) *.o $(LDFLAGS)

main:
	$(CC) $(CFLAGS) -c -o $(SRC)/$@.o $(SRC)/$@.c

client:
	$(CC) $(CFLAGS) -c -o $(SRC)/$@.o $(SRC)/$@.c

gui:
	$(CC) $(CFLAGS) -c -o $(SRC)/$@.o $(SRC)/$@.c

shapes:
	$(CC) $(CFLAGS) -c -o $(SRC)/$@.o $(SRC)/$@.c

gameplay:
	$(CC) $(CFLAGS) -c -o $(SRC)/$@.o $(SRC)/$@.c

server:
	$(CC) $(CFLAGS) -c -o $(SRC)/$@.o $(SRC)/$@.c	

run:
	$(SRC)/$(NAME) &$(SRC)/$(NAME)

clean:
	rm $(NAME) *.o *.exe

debug_build: CFLAGS = -g -Wall -Wextra
debug_build: build
debug_run: debug_build run
