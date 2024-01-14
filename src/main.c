#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <dlfcn.h> // POSIX only (dlopen, dlclose ...)
#include <raylib.h>

#include "logger.h"
#include "plug.h"

#define ARRAY_LEN(xs) sizeof(xs)/sizeof(xs[0])

const  char * libplug_file_name = "libplug.so";
       void * libplug = NULL;
plug_update_t plug_update = NULL;

char * shift_args(int * argc, char ***argv)
{
    assert(*argc > 0);
    if (*argc < 1) {
        log_error("No argument provided for the Music file.");
        exit(1);
    }
    char * result = (**argv);
    (*argv) += 1;
    (*argc) -= 1;
    return result;
}

// Hot reloading: links to libplug.so
bool reload_libplug(void)
{
    if (libplug != NULL) dlclose(libplug);

    libplug = dlopen(libplug_file_name, RTLD_NOW);
    if (libplug == NULL) {
        fprintf(stderr, "ERROR: could not load %s: %s\n", libplug_file_name, dlerror());
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

bool init(Plug * plug, const char * file_path)
{
    const float width = 800;
    const float height = 600;
    plug->height = height;
    plug->width = width;

    InitWindow(width, height, "Musializer");
    SetTargetFPS(60); // FPS set to 60 to stop flikering the sound, 30 for testing
    InitAudioDevice();

    // Load music
    plug->music = LoadMusicStream(file_path);

    // Check music
    if (! IsMusicReady(plug->music)) {
        log_error("Music is not ready");
        exit(1);
    }

    // Load font
    const int font_size = 35;
    const int glyph_count = 250;
    plug->font = LoadFontEx("./resources/fonts/NotoSans-Regular.ttf", font_size, 0, glyph_count);

    // Check font
    if (! IsFontReady(plug->font)) {
        log_error("Fonts not loaded");
        exit(1);
    }

    plug->music_len = GetMusicTimeLength(plug->music);
    plug->curr_volume = 0.5f;
    SetMusicVolume(plug->music, plug->curr_volume);
    ////AttachAudioStreamProcessor(plug->music.stream, callback);

    PlayMusicStream(plug->music); // For testing can remove later
    return true;
}

bool unload_and_close(Plug * plug)
{
    UnloadMusicStream(plug->music);
    UnloadFont(plug->font);
    CloseAudioDevice();
    CloseWindow();
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

    Plug plug = {0};

    init(&plug, file_path);

    while (! WindowShouldClose()) {
        // Quit/Close (Must be here not inside plug_update)
        if (IsKeyPressed(KEY_Q)) break;

        // Reload code (Must be here not inside plug_update)
        if (IsKeyPressed(KEY_R) && !reload_libplug()) return 1;

        plug_update(&plug);
    }

    unload_and_close(&plug);

    return 0;
}
