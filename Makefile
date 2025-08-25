ifeq ($(OS),Windows_NT)
	LDFLAGS += -lws2_32
else
	LDFLAGS += -Wall
endif

main: build
	./main

build:
	gcc src/*.c -o main $(LDFLAGS)

run:
	./main