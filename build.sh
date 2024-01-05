#! /usr/bin/env sh

# Build script to build musializer-clone app

# Set modes
#   -x (xtrace, trace mode, debug mode)
#   -e (exit script if any command errors - exits with non-zero status)
set -xe

# Define some C flags for the compiler
#   -Wall (Enable all warnings)
#   -Wextra (Enable more warnings no covered by -Wall)
CFLAGS="-Wall -Wextra"

clang $CFLAGS -o musializer main.c
