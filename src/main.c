#include <assert.h>
#include <complex.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <dlfcn.h> // POSIX only

#include "raylib.h"

#include "logger.h"
#include "plug.h"

#define ARRAY_LEN(xs) sizeof(xs)/sizeof(xs[0])

char * shift_args(int * argc, char ***argv)
{
    assert(*argc > 0);
    char * result = (**argv);
    (*argv) += 1;
    (*argc) -= 1;
    return result;
}

const char * libplug_file_name = "libplug.so";
void * libplug = NULL;
plug_hello_t plug_hello = NULL;
plug_init_t plug_init = NULL;
plug_update_t plug_update = NULL;
Plug plug = {0};

// Hot reloading: links to libplug.so
bool reload_libplug(void)
{
    if (libplug != NULL) dlclose(libplug);

    libplug = dlopen(libplug_file_name, RTLD_NOW);
    if (libplug == NULL) {
        fprintf(stderr, "ERROR: could not load %s: %s\n", libplug_file_name, dlerror());
        return false;
    }

    plug_hello = dlsym(libplug, "plug_hello");
    if (plug_hello == NULL) {
        fprintf(stderr, "ERROR: could not find plug_hello symbol in %s: %s\n", libplug_file_name, dlerror());
        return false;
    }

    plug_init = dlsym(libplug, "plug_init");
    if (plug_init == NULL) {
        fprintf(stderr, "ERROR: could not find plug_init symbol in %s: %s\n", libplug_file_name, dlerror());
        return false;
    }

    plug_update = dlsym(libplug, "plug_update");
    if (plug_update == NULL) {
        fprintf(stderr, "ERROR: could not find plug_update symbol in %s: %s\n", libplug_file_name, dlerror());
        return false;
    }

    log_info("libplug.so Reloaded");

    return true;
}

int main(int argc, char **argv)
{
    if (!reload_libplug()) return 1;

    const char * program = shift_args(&argc, &argv);
    // TODO: supply input file vie drag & drop
    if (argc == 0) {
        fprintf(stderr, "Usage: %s <input>\n", program);
        fprintf(stderr, "ERROR: no input file is provided\n");
        return 1;
    }
    const char * file_path = shift_args(&argc, &argv);

    InitWindow(800, 600, "Musializer");
    SetTargetFPS(30); // FPS set to 60 to stop flikering the sound, 30 for testing
    InitAudioDevice();

    plug_init(&plug, file_path);
    while (! WindowShouldClose()) {
        if (IsKeyPressed(KEY_R) && !reload_libplug()) return 1;
        if (IsKeyPressed(KEY_Q)) break; // Quit/Close
        plug_update(&plug);
    }

    UnloadMusicStream(plug.music);
    //UnloadFont(plug.font);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
