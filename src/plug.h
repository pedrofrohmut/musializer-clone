#ifndef PLUG_H_
#define PLUG_H_

#include <complex.h>
#include <raylib.h>

#define C_DARK_GRAY     CLITERAL(Color){ 0x23, 0x23, 0x23, 0xFF } // Dark  Gray
#define C_LIGHT_GRAY    CLITERAL(Color){ 0xCC, 0xCC, 0xCC, 0xFF } // Light Gray
#define C_MATRIX_GREEN  CLITERAL(Color){ 0x66, 0xFF, 0x33, 0xFF } // Matrix Green
#define C_MATRIX_PURPLE CLITERAL(Color){ 0x99, 0x00, 0xCC, 0xFF } // Matrix Purple

const int W_WIDTH = 800; // 800
const int W_HEIGHT = 600; // 600

const Color BACKGROUND_COLOR = C_DARK_GRAY;
const Color TEXT_COLOR = C_LIGHT_GRAY;
const Color RECT_COLOR = C_MATRIX_GREEN;
const Color RECT_NEG_COLOR = C_MATRIX_PURPLE;

const float TXT_SPACING = 2.0f;

#define N 256

typedef struct {
    float left;
    float right;
} Frame;

typedef struct {
    float curr_volume;
    float music_len;
    Music music;
    Font font;
} Plug;

typedef void (*plug_hello_t)(void); // Type alias

// TODO: Leave INIT out of if. Since all that matters is the loop anyway
// Initialize the state
typedef void (*plug_init_t)(Plug * plug, const char * file_path);

// Run on every loop
typedef void (*plug_update_t)(Plug * plug);

#endif//PLUG_H_
