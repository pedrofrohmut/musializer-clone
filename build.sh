#! /usr/bin/env sh

# Build script to build musializer-clone app

# Raylib instructions: Working on GNU Linux
# https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux
# DOCS Cmd: $ cc main.c -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

# Set modes
#   -x (xtrace, trace mode, debug mode)
#   -e (exit script if any command errors - exits with non-zero status)
set -xe

# Define some C flags for the compiler
#   -Wall (Enable all warnings)
#   -Wextra (Enable more warnings no covered by -Wall)
CFLAGS="-Wall -Wextra `pkg-config --cflags raylib`"

LIBS="`pkg-config --libs raylib` -lraylib -lglfw -lm -ldl -lpthread"

SOURCE_FILE=main.c
OUTPUT_FILE=musializer.out

gcc $CFLAGS -o $OUTPUT_FILE $SOURCE_FILE $LIBS

echo -e "\n[SUCCESS] Compiled projects successfully into $OUTPUT_FILE"
