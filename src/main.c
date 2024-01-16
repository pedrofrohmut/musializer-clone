#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <dlfcn.h> // POSIX only (dlopen, dlclose ...)
#include <raylib.h>
#include <math.h>

#include "logger.h"
#include "plug.h"

#define ARRAY_LEN(xs) sizeof(xs)/sizeof(xs[0])

// Libplug: Must be a global variable to work (static lifetime)
                           void * libplug = NULL;
                plug_reload_t plug_reload = NULL;
                plug_update_t plug_update = NULL;
plug_audio_callback_t plug_audio_callback = NULL;

// Hot reloading: links to libplug.so
bool reload_libplug(PlugState * plug)
{
    if (plug != NULL
            && IsAudioStreamReady(plug->music.stream)
            && plug_audio_callback != NULL) {
        // Detach the audio from the previous version of the callback function
        DetachAudioStreamProcessor(plug->music.stream, plug_audio_callback);
    }

    const char * libplug_file_name = "libplug.so";

    if (libplug != NULL) dlclose(libplug);

    libplug = dlopen(libplug_file_name, RTLD_NOW);
    if (libplug == NULL) {
        fprintf(stderr, "ERROR: could not load %s: %s\n", libplug_file_name, dlerror());
        return false;
    }

    plug_reload = dlsym(libplug, "plug_reload");
    if (plug_reload == NULL) {
        fprintf(stderr, "ERROR: could not find plug_reload symbol in %s: %s\n", libplug_file_name, dlerror());
        return false;
    }

    plug_update = dlsym(libplug, "plug_update");
    if (plug_update == NULL) {
        fprintf(stderr, "ERROR: could not find plug_update symbol in %s: %s\n", libplug_file_name, dlerror());
        return false;
    }

    plug_audio_callback = dlsym(libplug, "plug_audio_callback");
    if (plug_audio_callback == NULL) {
        fprintf(stderr, "ERROR: could not find plug_audio_callback symbol in %s: %s\n", libplug_file_name, dlerror());
        return false;
    }

    // Reloads internal state of plug
    plug_reload(plug);

    if (plug != NULL && IsAudioStreamReady(plug->music.stream)) {
        // Attach the audio again to the new version
        AttachAudioStreamProcessor(plug->music.stream, plug_audio_callback);
    }

    log_info("libplug.so Reloaded");
    return true;
}

size_t calculate_m(PlugState * plug)
{
    size_t count = 0;
    float freq = 20.0f;
    while (freq < plug->n) {
        freq = freq * plug->step;
        count++;
    }
    return count;
}

void main_init(PlugState * plug, const char * file_path)
{
    plug->width = 800;
    plug->height = 600;
    plug->n = (size_t) 2 << 13; // about 16k
    plug->step = 1.06f;
    plug->m = calculate_m(plug);
    plug->in = (float *) calloc(plug->n, sizeof(float));
    plug->out = (float complex *) calloc(plug->n, sizeof(float complex));

    InitWindow(plug->width, plug->height, "Musializer");
    SetTargetFPS(30); // FPS set to 60 to stop flikering the sound, 30 for testing
    InitAudioDevice();

    // Load music
    plug->music = LoadMusicStream(file_path);

    // Check music
    if (! IsMusicReady(plug->music)) {
        fprintf(stderr, "Music is not ready");
        exit(1);
    }

    plug->music_len = GetMusicTimeLength(plug->music);
    plug->curr_volume = 0.0f;
    SetMusicVolume(plug->music, plug->curr_volume);
    AttachAudioStreamProcessor(plug->music.stream, plug_audio_callback);
    PlayMusicStream(plug->music); // For testing can remove later

    // Load font
    const int font_size = 35;
    const int glyph_count = 250;
    plug->font = LoadFontEx("./resources/fonts/NotoSans-Regular.ttf", font_size, 0, glyph_count);

    // Check font
    if (! IsFontReady(plug->font)) {
        fprintf(stderr, "Fonts not loaded");
        exit(1);
    }
}

void unload_and_close(PlugState * plug)
{
    // Raylib
    UnloadMusicStream(plug->music);
    UnloadFont(plug->font);
    CloseAudioDevice();
    CloseWindow();

    // Plug lib and state
    free(plug->in);
    free(plug->out);

    // Close handle for libplug
    dlclose(libplug);
}

char * shift_args(int * argc, char ***argv)
{
    if (*argc < 1) {
        fprintf(stderr, "No argument provided for the Music file.");
        exit(1);
    }
    char * result = (**argv);
    (*argv) += 1;
    (*argc) -= 1;
    return result;
}

const char * get_file_path(int argc, char ** argv)
{
    const char * program = shift_args(&argc, &argv);
    // TODO: supply input file vie drag & drop
    if (argc == 0) {
       fprintf(stderr, "Usage: %s <input>\n", program);
       fprintf(stderr, "ERROR: no input file is provided\n");
       exit(1);
    }
    const char * file_path = shift_args(&argc, &argv);
    return file_path;
}

int main(int argc, char **argv)
{
    PlugState plug;

    if (!reload_libplug(&plug)) {
        fprintf(stderr, "Could not load libplug.so. That is required for This app to work.\n");
        return 1;
    }

    const char * file_path = get_file_path(argc, argv);

    main_init(&plug, file_path);
    plug_reload(&plug); // Reload state after values have been initialized on main_init

    while (! WindowShouldClose()) {
        // Quit/Close (Must be here not inside plug_update)
        if (IsKeyPressed(KEY_Q)) break;

        // Hot Reload the code from libplug.so (Must be here not inside plug_update)
        if (IsKeyPressed(KEY_R) && !reload_libplug(&plug)) return 1;

        plug_update(&plug);
    }

    unload_and_close(&plug);

    return 0;
}
