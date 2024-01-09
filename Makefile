CC = gcc
CFLAGS = -Wall -Wextra -std=c99 $(pkg-config --cflags raylib)
LIBS = $(pkg-config --libs raylib) -lraylib -lglfw -lm -ldl -lpthread

SOURCE_FILE = src/main.c
OUTPUT_FILE = musializer.out
MY_BINARIES = bin/logger.o

all: clean logger main

logger: src/logger.c
	${CC} ${CFLAGS} -c src/logger.c -o bin/logger.o
	@echo "Logger built into binaries"

main: src/main.c
	${CC} ${CFLAGS} -o ${OUTPUT_FILE} ${SOURCE_FILE} ${MY_BINARIES} ${LIBS}
	@echo "Muzializer built with no errors"

clean:
	rm -f bin/*.o
	rm -f musializer.out
	@echo "Clean up complete"
