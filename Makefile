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

LIBS = $(pkg-config --libs raylib) -lraylib -lglfw -lm -ldl -lpthread -L./build/

all: clean dist

dev: plug_dev main_dev

debug: logger plug main_debug

dist: plug_dist main_dist

clean:
	rm -f bin/*.o
	rm -f build/*.so
	rm -f build/*.out
	@echo -e "OK > Clean up complete\n"

### LIBS ###########################################################################################

# -fPIC -shared are the flag tha makes the output into a shared library
plug: src/plug.c
	${CC} ${CFLAGS} -o build/libplug.so -fPIC -shared src/plug.c ./bin/logger.o ${LIBS}
	@echo -e "OK > build/libplug.so built with no errors\n"

logger: src/logger.c
	${CC} ${CFLAGS} -c src/logger.c -o bin/logger.o
	@echo -e "OK > bin/logger.o built into binaries\n"

### DEV ############################################################################################

# -fPIC -shared are the flag tha makes the output into a shared library
plug_dev: src/plug.c
	${CC} ${CFLAGS} -DDEV_ENV -o build/libplug.so -fPIC -shared src/plug.c ./bin/logger.o ${LIBS}
	@echo -e "OK > build/libplug.so built with no errors\n"

main_dev: src/main.c
	${CC} ${CFLAGS} -DDEV_ENV -o ./build/dev.out ./src/main.c ./bin/logger.o ${LIBS}
	@echo -e "OK > build/dev.out built with no errors"

### DEBUG ##########################################################################################

# ggdb: debug info for gdb, -Og: Optimization made for debug, -Werror: treat warnings as errors
main_debug: src/main.c
	${CC} ${CFLAGS} -ggdb -Werror -Og -o ./build/debug.out ./src/main.c ./bin/logger.o ${LIBS}
	@echo -e "OK > build/debug.out built with no errors"

### DISTRIBUTION/PRODUCTION ########################################################################

# -fPIC -shared are the flag tha makes the output into a shared library
plug_dist: src/plug.c
	${CC} ${CFLAGS} -o build/libplug.so -fPIC -shared ./src/plug.c ./src/logger.c ${LIBS}
	@echo -e "OK > build/libplug.so built with no errors\n"

# Static link with plug and logger
main_dist:
	${CC} ${CFLAGS} -o ./build/musializer.out ./src/plug.c ./src/logger.c ./src/main.c ${LIBS}
	@echo -e "OK > build/muzializer.out built with no errors"
