#ifndef PLUG_H_
#define PLUG_H_

#include <complex.h>
#include <raylib.h>

typedef struct {
    float left;          // Sample for left channel
    float right;         // Sample for right channel
} Frame;

typedef struct {
    float width;         // Window width
    float height;        // Window height

    Font font;           // Font loaded to be used on drawing

    float curr_volume;   // Music current volume
    float music_len;     // Music total length
    Music music;         // Main music

    float * in;          // Input buffer for audio samples (left channel)
    float complex * out; // Output buffer for FFT
    size_t n;            // The size of input and output buffers
    size_t in_size;      // Track filled part of input buffer

    size_t m;            // Number of frequencies in the interval
    float step;          // Constant from Frequency Table Formula

    unsigned int skip_c; // Counter to skip frames
} PlugState;

// Run on every loop
typedef void (*plug_update_t)(PlugState * plug);

// Attached to the music stream
typedef void (*plug_audio_callback_t)(void * dataBuffer, unsigned int frames);

// Reload the global variables for input and output
typedef void (*plug_reload_t)(PlugState * plug);

#endif//PLUG_H_
