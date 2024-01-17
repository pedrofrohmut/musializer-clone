#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <dlfcn.h> // POSIX only (dlopen, dlclose ...)
#include <raylib.h>
#include <math.h>

#include "logger.h"
#include "plug.h"

#define ARRAY_LEN(xs) sizeof(xs)/sizeof(xs[0])

// TODO: make the PlugState a global var of libplug.so
// The plug_init creates it and returns a point to main
// main can return the pointer to libplug on reload
// > The state will belong to libplug.so and main will just hold the reference
// and pass it back on plug_reload
// > The instance will be created at plug_init and returned to main
// > This wont change much functionality. but you wont have to places with the same
// value.

// TODO: maybe refactor all plug function pointers to be part of PlugState

// Libplug: Must be a global variable to work (static lifetime)
static void * libplug = NULL;


// This macro auto generates the variables for the functions. You just add to plug.h List Macro
#ifdef HOT_RELOAD

#define PLUG(name) static name##_t * name = NULL;
LIST_OF_PLUGS
#undef PLUG

#else

#define PLUG(name) static name##_t name;
LIST_OF_PLUGS
#undef PLUG

#endif

/*                 static plug_reload_t plug_reload = NULL; */
/*                 static plug_update_t plug_update = NULL; */
/* static plug_audio_callback_t plug_audio_callback = NULL; */

#ifdef HOT_RELOAD
void pre_reload_libplug(PlugState * plug)
{
    if (plug != NULL && IsMusicReady(plug->music) && plug_audio_callback != NULL) {
        // Detach the audio from the previous version of the callback function
        DetachAudioStreamProcessor(plug->music.stream, plug_audio_callback);
    }
}

void post_reload_libplug(PlugState * plug)
{
    // Reloads internal state of plug
    plug_reload(plug);

    if (plug != NULL && IsMusicReady(plug->music) && plug_audio_callback != NULL) {
        // Attach the audio again to the new version
        AttachAudioStreamProcessor(plug->music.stream, plug_audio_callback);
    }
}

// Hot reloading: links to libplug.so
bool reload_libplug(PlugState * plug)
{
    pre_reload_libplug(plug);

    const char * libplug_file_name = "libplug.so";

    if (libplug != NULL) dlclose(libplug);

    libplug = dlopen(libplug_file_name, RTLD_NOW);
    if (libplug == NULL) {
        fprintf(stderr, "ERROR: could not load %s: %s\n", libplug_file_name, dlerror());
        return false;
    }

    // This macro makes the dlsym of all the functions that is just copy/pasta anyway
    #define PLUG(name) \
    name = dlsym(libplug, #name); \
    if (name == NULL) { \
        fprintf(stderr, "ERROR: could not find name symbol in %s: %s\n", libplug_file_name, dlerror()); \
        return false; \
    }
    LIST_OF_PLUGS
    #undef PLUG

    log_info("libplug.so Reloaded");

    post_reload_libplug(plug);

    return true;
}
#else
bool reload_libplug(PlugState * plug) { return true; }
#endif

size_t calculate_m(const size_t n, const float step)
{
    size_t count = 0;
    float freq = 20.0f;
    while (freq < n) {
        freq = freq * step;
        count++;
    }
    return count;
}

void main_init(PlugState * plug, const char * file_path)
{
    // Window
    plug->width = 800;
    plug->height = 600;

    // Input/Output buffers
    plug->n = (size_t) 2 << 13; // 2 << 13 == 16,384 (13 is default, min is 9)
    plug->in = (float *) calloc(plug->n, sizeof(float));
    plug->out = (float complex *) calloc(plug->n, sizeof(float complex));
    plug->in_size = 0;

    // Calculate frequencies
    plug->step = 1.06f;
    plug->m = calculate_m(plug->n, plug->step);

    // Skip frames
    plug->skip_c = 0;

    InitWindow(plug->width, plug->height, "Musializer");
    SetTargetFPS(60); // FPS set to 60 to stop flikering the sound, 30 for testing
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
    DetachAudioStreamProcessor(plug->music.stream, plug_audio_callback);
    UnloadMusicStream(plug->music);
    UnloadFont(plug->font);
    CloseAudioDevice();
    CloseWindow();

    // Plug lib and state
    free(plug->out);

    // Close handle for libplug
    if (libplug != NULL) dlclose(libplug);
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

// TODO: Write my own Attach Detach Functions that accept callback like:
// typedef void (* audio_callback_t)(void * bufferData, unsigned int frames_c, PlugState * plug)
// typedef void (*AudioCallback)(void *bufferData, unsigned int frames);
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
