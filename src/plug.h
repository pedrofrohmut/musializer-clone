#ifndef PLUG_H_
#define PLUG_H_

#include <complex.h>
#include <raylib.h>

#define C_DARK_GRAY     CLITERAL(Color){ 0x23, 0x23, 0x23, 0xFF } // Dark  Gray
#define C_LIGHT_GRAY    CLITERAL(Color){ 0xCC, 0xCC, 0xCC, 0xFF } // Light Gray
#define C_MATRIX_GREEN  CLITERAL(Color){ 0x66, 0xFF, 0x33, 0xFF } // Matrix Green
#define C_MATRIX_PURPLE CLITERAL(Color){ 0x99, 0x00, 0xCC, 0xFF } // Matrix Purple

const Color BACKGROUND_COLOR = C_DARK_GRAY;
const Color TEXT_COLOR       = C_LIGHT_GRAY;
const Color RECT_COLOR       = C_MATRIX_GREEN;
const Color RECT_NEG_COLOR   = C_MATRIX_PURPLE;

const float TEXT_SPACING = 2.0f;

#define N 256

typedef struct {
    float left;
    float right;
} Frame;

typedef struct {
    // Window
    float height;
    float width;

    // Music
    float curr_volume;
    float music_len;
    Music music;

    // Audio Samples
    float * in;
    float complex * out;

    // Font
    Font font;
} Plug;

// Run on every loop
typedef void (*plug_update_t)(Plug * plug);

// Attached to the music stream
typedef void (*plug_audio_callback_t)(void * dataBuffer, unsigned int frames);

// Reload the global variables for input and output
typedef void (*plug_reload_t)(Plug * plug);

#endif//PLUG_H_
