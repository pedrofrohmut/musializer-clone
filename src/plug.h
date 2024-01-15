#ifndef PLUG_H_
#define PLUG_H_

#include <complex.h>
#include <raylib.h>

#define N 256

typedef struct {
    float left;  // Sample for left channel
    float right; // Sample for right channel
} Frame;

typedef struct {
    float height;        // Window height
    float width;         // Window width

    float curr_volume;   // Music current volume
    float music_len;     // Music total length
    Music music;         // Main music

    float * in;          // Input buffer
    float complex * out; // Output buffer

    Font font;           // Font loaded to be used on drawing
} Plug;

// Run on every loop
typedef void (*plug_update_t)(Plug * plug);

// Attached to the music stream
typedef void (*plug_audio_callback_t)(void * dataBuffer, unsigned int frames);

// Reload the global variables for input and output
typedef void (*plug_reload_t)(Plug * plug);

#endif//PLUG_H_
