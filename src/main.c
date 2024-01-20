#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <dlfcn.h> // POSIX only (dlopen, dlclose ...)
#include <raylib.h>
#include <math.h>
#include <string.h>

#include "logger.h"
#include "plug.h"

#define ARRAY_LEN(xs) sizeof(xs)/sizeof(xs[0])

// TODO: maybe refactor all plug function pointers to be part of PlugState

// Libplug: Must be a global variable to work (static lifetime)
static void * libplug_handle = NULL;

// Plug methods
static plug_reload_t         plug_reload         = NULL;
static plug_update_t         plug_update         = NULL;
static plug_audio_callback_t plug_audio_callback = NULL;
static plug_load_music_t     plug_load_music     = NULL;
static plug_set_playing_t    plug_set_playing    = NULL;

void pre_reload_libplug(PlugState * plug)
{
    if (plug != NULL && IsMusicReady(plug->music) && plug_audio_callback != NULL) {
        DetachAudioStreamProcessor(plug->music.stream, plug_audio_callback);
    }
}

void post_reload_libplug(PlugState * plug)
{
    plug_reload(plug);
    if (plug != NULL && IsMusicReady(plug->music) && plug_audio_callback != NULL) {
        AttachAudioStreamProcessor(plug->music.stream, plug_audio_callback);
    }
}

// Hot reloading: links to libplug.so
bool reload_libplug(PlugState * plug)
{
    pre_reload_libplug(plug);

    const char * libplug_file_name = "libplug.so";

    if (libplug_handle != NULL) dlclose(libplug_handle);

    libplug_handle = dlopen(libplug_file_name, RTLD_NOW);
    if (libplug_handle == NULL) {
        fprintf(stderr, "ERROR: could not load %s: %s\n", libplug_file_name, dlerror());
        return false;
    }

    plug_reload = dlsym(libplug_handle, "plug_reload");
    if (plug_reload == NULL) {
        fprintf(stderr, "ERROR: could not find plug_reload symbol in %s: %s\n", libplug_file_name, dlerror());
        return false;
    }

    plug_update = dlsym(libplug_handle, "plug_update");
    if (plug_update == NULL) {
        fprintf(stderr, "ERROR: could not find plug_update symbol in %s: %s\n", libplug_file_name, dlerror());
        return false;
    }

    plug_audio_callback = dlsym(libplug_handle, "plug_audio_callback");
    if (plug_audio_callback == NULL) {
        fprintf(stderr, "ERROR: could not find plug_audio_callback symbol in %s: %s\n", libplug_file_name, dlerror());
        return false;
    }

    plug_load_music = dlsym(libplug_handle, "plug_load_music");
    if (plug_load_music == NULL) {
        fprintf(stderr, "ERROR: could not find plug_load_music symbol in %s: %s\n", libplug_file_name, dlerror());
        return false;
    }

    plug_set_playing = dlsym(libplug_handle, "plug_set_playing");
    if (plug_set_playing == NULL) {
        fprintf(stderr, "ERROR: could not find plug_set_playing symbol in %s: %s\n", libplug_file_name, dlerror());
        return false;
    }

    post_reload_libplug(plug);

    log_info("libplug.so Reloaded");
    return true;
}

// TODO: checks later if this is still needed
size_t calculate_m(const size_t n, const float step)
{
    /* size_t count = 0; */
    /* float freq = 20.0f; */
    /* while (freq < n) { */
    /*     freq = freq * step; */
    /*     count++; */
    /* } */
    /* return count; */
    size_t m = 0; // M frequencies
    const float LOWF = 1.0f; // TODO: make it an arg
    for (float f = LOWF; (size_t) f < n/2; f = ceilf(f * step)) m++;
    return m;
}

void main_init(PlugState * plug, const char * file_path)
{
    // Window
    plug->width = 800;
    plug->height = 600;

    // Input/Output buffers
    plug->n = (size_t) 2 << 9; // 2 << 13 == 16,384 (13 is default, min is 9)
    plug->in1 = (float *) calloc(plug->n, sizeof(float));
    plug->in2 = (float *) calloc(plug->n, sizeof(float));
    plug->out = (float complex *) calloc(plug->n, sizeof(float complex));
    plug->in_size = 0;

    // Calculate frequencies
    plug->step = 1.06f;
    plug->m = calculate_m(plug->n, plug->step);

    // Skip frames
    plug->skip_c = 0;

    // UI strings
    strncpy(plug->str.title, "Musializer", sizeof(plug->str.title));
    strncpy(plug->str.drag_txt, "Drag & Drop Music Files Here", sizeof(plug->str.drag_txt));
#ifdef DEV_ENV // String to print N on dev mode
    snprintf(plug->str.n_str, sizeof(plug->str.n_str), "%zu", plug->n);
#endif

    // Error
    strncpy(plug->error.message, "", sizeof(plug->error.message));
    plug->error.has_error = false;

    InitWindow(plug->width, plug->height, "Musializer");
    SetTargetFPS(60); // FPS set to 60 to stop flikering the sound, 30 for testing
    InitAudioDevice();

    if (file_path != NULL) {
        plug_load_music(plug, file_path);
        plug->curr_volume = 0.0f;
        SetMusicVolume(plug->music, plug->curr_volume);
        AttachAudioStreamProcessor(plug->music.stream, plug_audio_callback);
        PlayMusicStream(plug->music); // For testing can remove later
        plug_set_playing(plug, true);
    }

    // Load font
    const int font_size = 30;
    const int glyph_count = 250;
    plug->font = LoadFontEx("./resources/fonts/NotoSans-Regular.ttf", font_size, 0, glyph_count);

    // Check font
    if (! IsFontReady(plug->font)) {
        fprintf(stderr, "Fonts not loaded");
        exit(1);
    }

    log_info("main app initialized");
}

void unload_and_close(PlugState * plug)
{
    // Raylib
    if (IsMusicReady(plug->music)) {
        DetachAudioStreamProcessor(plug->music.stream, plug_audio_callback);
        UnloadMusicStream(plug->music);
    }
    UnloadFont(plug->font);
    CloseAudioDevice();
    CloseWindow();

    // Close handle for libplug
    if (libplug_handle != NULL) dlclose(libplug_handle);
}

// TODO: Write my own Attach Detach Functions that accept callback like:
// typedef void (* audio_callback_t)(void * bufferData, unsigned int frames_c, PlugState * plug)
// typedef void (*AudioCallback)(void *bufferData, unsigned int frames);
int main(int argc, char **argv)
{
    (void) argc;

    PlugState plug;

    if (!reload_libplug(&plug)) {
        fprintf(stderr, "Could not load libplug.so. That is required for This app to work.\n");
        return 1;
    }

    const char * file_path = argv[1];
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
