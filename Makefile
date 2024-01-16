# Makefile for musializer-clone app

# Raylib instructions: Working on GNU Linux
# https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux
# DOCS Cmd: $ cc main.c -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

# ENV

export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/lib/pkgconfig:$HOME/.pkgconfig
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib:./build

# Compiler app
CC = gcc

# Define some C flags for the compiler
#   -Wall (Enable all warnings)
#   -Wextra (Enable more warnings no covered by -Wall)
CFLAGS = -Wall -Wextra -std=c99 $(pkg-config --cflags raylib)

LIBS = $(pkg-config --libs raylib) -lraylib -lglfw -lm -ldl -lpthread  -L./build/

all: clean logger plug main

clean:
	rm -f bin/*.o
	rm -f build/*.so
	rm -f build/*.out
	@echo -e "OK > Clean up complete\n"

logger: src/logger.c
	${CC} ${CFLAGS} -c src/logger.c -o bin/logger.o
	@echo -e "OK > bin/logger.o built into binaries\n"

plug: src/plug.c
	${CC} ${CFLAGS} -o build/libplug.so -fPIC -shared src/plug.c ./bin/logger.o ${LIBS}
	@echo -e "OK > build/libplug.so built with no errors\n"

main: src/main.c
	${CC} ${CFLAGS} -o ./build/musializer.out ./src/main.c ./bin/logger.o ${LIBS}
	@echo -e "OK > build/muzializer.out built with no errors"

debug: src/main.c
	${CC} ${CFLAGS} -g -Werror -O0 -o ./build/debug_musializer.out ./src/main.c ./src/logger.c ${LIBS}
	@echo -e "OK > build/debug_muzializer.out built with no errors"
