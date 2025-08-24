ifeq ($(OS),Windows_NT)
	LDFLAGS += -lws2_32
else
	LDFLAGS += -Wall
endif

run: main
	./main

main:
	gcc main.c server.c -o main $(LDFLAGS)