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

all: clean main_dist

dev: app_dev main_dev

debug: logger_debug app_debug main_debug

dist: main_dist

clean:
	rm -f bin/*.o
	rm -f build/*.so
	rm -f build/*.out
	@echo -e "OK > Clean up complete\n"

### DEV ############################################################################################

logger_dev: src/logger.c
	${CC} ${CFLAGS} -DDEV_ENV -o ./bin/dev_logger.o -c ./src/logger.c
	@echo -e "OK > bin/dev_logger.o built into binaries\n"

app_dev: src/app.c
	${CC} ${CFLAGS} -DDEV_ENV -o ./bin/dev_app.o -c ./src/app.c ${LIBS}
	@echo -e "OK > bin/dev_app.o built into binaries\n"

main_dev: src/main.c
	${CC} ${CFLAGS} -DDEV_ENV -o ./build/dev.out ./src/main.c ./bin/dev_app.o ./bin/dev_logger.o ${LIBS}
	@echo -e "OK > build/dev.out built with no errors"

### DEBUG ##########################################################################################

logger_debug: src/logger.c
	${CC} ${CFLAGS} -DDEV_ENV -ggdb -Werror -Og -o ./bin/debug_logger.o -c ./src/logger.c
	@echo -e "OK > bin/debug_logger.o built into binaries\n"

app_debug: src/app.c
	${CC} ${CFLAGS} -DDEV_ENV -ggdb -Werror -Og -o ./bin/debug_app.o -c ./src/app.c ${LIBS}
	@echo -e "OK > bin/debug_app.o built into binaries\n"

# ggdb: debug info for gdb, -Og: Optimization made for debug, -Werror: treat warnings as errors
main_debug: src/main.c
	${CC} ${CFLAGS} -DDEV_ENV -ggdb -Werror -Og -o ./build/debug.out ./src/main.c ./bin/debug_app.o ./bin/debug_logger.o ${LIBS}
	@echo -e "OK > build/debug.out built with no errors"

### DISTRIBUTION/PRODUCTION ########################################################################

# Static link with app and logger
main_dist:
	${CC} ${CFLAGS} -o ./build/musializer.out ./src/app.c ./src/logger.c ./src/main.c ${LIBS}
	@echo -e "OK > build/muzializer.out built with no errors"

### EXTRA ##########################################################################################

foo: ./extra/foo.c
	${CC} ${CFLAGS} -o ./build/foo.out ./extra/foo.c -lm
	@echo "OK > build/foo.out built with no errors"
