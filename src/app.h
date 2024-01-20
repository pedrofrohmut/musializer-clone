#ifndef APP_H_
#define APP_H_

#include <complex.h>
#include <raylib.h>

#define MAX_STRING_LENGHT 100

typedef struct {
    char title[MAX_STRING_LENGHT];
    char vol_time[MAX_STRING_LENGHT];
    char play_state[MAX_STRING_LENGHT];
    char n_str[MAX_STRING_LENGHT];
    char drag_txt[MAX_STRING_LENGHT];
} AppStrings;

typedef struct {
    bool has_error;      // Error state
    char message[1024];      // Error message
} AppError;

typedef struct {
    float width;         // Window width
    float height;        // Window height

    Font font;           // Font loaded to be used on drawing

    float curr_volume;   // Music current volume
    float curr_time;
    float music_len;     // Music total length
    Music music;         // Main music

    float * in1;          // Input buffer for audio samples (left channel)
    float * in2;
    float complex * out; // Output buffer for FFT
    size_t n;            // The size of input and output buffers
    size_t in_size;      // Track filled part of input buffer

    float samples[1024]; // samples data arr for the audio callback

    float lowf;          // The low frequency that is the base for calculations
    float step;          // Constant from Frequency Table Formula
    size_t m;            // Number of frequencies in the interval

    unsigned int skip_c; // Counter to skip frames

    AppStrings str;     // Holds the string to UI

    AppError error;     // Holds error state and message
} AppState;

AppState * app_init(const char * file_path);

void app_update(AppState * state);

void app_draw(AppState * state);

void app_unload_and_close(AppState * state);

#endif // APP_H_
