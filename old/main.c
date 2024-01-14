#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

//#include <string.h>
//#include <complex.h>
//#include <math.h>
//#include <stdint.h>

#include <unistd.h> // Check file exists

#include <dlfcn.h> // POSIX only (dlopen, dlclose ...)

#include "raylib.h"

#include "logger.h"
#include "plug.h"

#define ARRAY_LEN(xs) sizeof(xs)/sizeof(xs[0])

const char * libplug_file_name = "libplug.so";
void * libplug = NULL;
plug_hello_t plug_hello = NULL;
plug_init_t plug_init = NULL;
plug_update_t plug_update = NULL;
Plug g_plug;

char * shift_args(int * argc, char ***argv)
{
    assert(*argc > 0);
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

bool init(Plug * plug, const char * file_path)
{

    InitWindow(800, 600, "Musializer");
    SetTargetFPS(30); // FPS set to 60 to stop flikering the sound, 30 for testing
    InitAudioDevice();

    /* const int music_exists = access(file_path, F_OK); // -1 for file not found */
    /* if (music_exists == -1) { */
    /*     printf("Music file not found in the path provided\n"); */
    /* } */

    // Load music
    plug->music = LoadMusicStream(file_path);

    // Check music
    if (IsMusicReady(plug->music)) {
        log_debug("Music is ready (Why?)");
        UnloadMusicStream(plug->music);
        //DetachAudioStreamProcessor(plug->music.stream, audio_callback);
    } else {
        log_debug("Music is NOT ready (normal)");
    }

    // Load font
    const int font_size = 35;
    const int glyph_count = 250;
    plug->font = LoadFontEx("./resources/fonts/NotoSans-Regular.ttf", font_size, 0, glyph_count);

    // Check font
    if (!IsFontReady(plug->font)) {
        log_error("Fonts not loaded");
        exit(1);
    }

    plug->music_len = GetMusicTimeLength(plug->music);
    plug->curr_volume = 0.5f;
    SetMusicVolume(plug->music, plug->curr_volume);
    AttachAudioStreamProcessor(plug->music.stream, callback);

    PlayMusicStream(plug->music); // For testing can remove later
    return true;
}

bool unload_and_close()
{
    UnloadMusicStream(g_plug.music);
    UnloadFont(g_plug.font);
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

    init(&g_plug, file_path);

    plug_init(&g_plug, file_path);
    while (! WindowShouldClose()) {
        if (IsKeyPressed(KEY_R)) {
            if (!reload_libplug())
                return 1;
            else
                printf("RELOADED!\n");
        }

        if (IsKeyPressed(KEY_Q)) break; // Quit/Close

        plug_update(&g_plug);
    }

    unload_and_close();

    return 0;
}
