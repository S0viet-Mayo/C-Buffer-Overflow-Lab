# Makes buffy buffer overflow project

CC = gcc

all: buffy

buffy: input
	@echo "Compiling..."
	${CC} -o buffy main.o input.o

input:
	@echo "Compiling..."
	${CC} -c input.c

clean:
	rm input.o
